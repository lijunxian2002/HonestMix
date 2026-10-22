#include "UI/Skin.h"
#include <cmath>
#include <map>
#include <vector>

namespace hm::skin
{

//==============================================================================
// Fonts
//==============================================================================
juce::Font uiFont (float logicalHeight, bool bold)
{
    static const auto families = juce::Font::findAllTypefaceNames();
    static const juce::String family = families.contains ("Segoe UI", true)
        ? "Segoe UI" : juce::Font::getDefaultSansSerifFontName();
    return juce::Font (family, juce::jmax (8.0f, logicalHeight),
                       bold ? juce::Font::bold : juce::Font::plain);
}

void drawText (juce::Graphics& g, const juce::String& s, juce::Rectangle<int> r,
               float logicalHeight, juce::Colour c, juce::Justification align, bool bold)
{
    bool cjk = false;
    for (auto ch : s) if ((unsigned int)ch >= 0x2e80u) { cjk = true; break; }
    g.setFont (cjk ? juce::Font ("Microsoft YaHei UI", juce::jmax(8.f, logicalHeight),
                                bold ? juce::Font::bold : juce::Font::plain)
                   : uiFont (logicalHeight, bold));
    g.setColour (c);
    g.drawFittedText (s, r, align, 1, 1.0f);
}

//==============================================================================
// Scale aware vector primitives
//==============================================================================
float& drawingScaleRef()
{
    static thread_local float scale = 1.0f;
    return scale;
}

void setDrawingScale (float scale)
{
    drawingScaleRef() = (scale > 0.01f) ? scale : 1.0f;
}

float drawingScale()
{
    return drawingScaleRef();
}

float deviceScale (juce::Graphics& g)
{
    return juce::jmax (drawingScaleRef(), g.getInternalContext().getPhysicalPixelScaleFactor());
}

void strokeCrisp (juce::Graphics& g, const juce::Path& p, float logicalWidth,
                  juce::Colour c, juce::PathStrokeType::JointStyle joint,
                  juce::PathStrokeType::EndCapStyle cap)
{
    const float w = juce::jmax (logicalWidth, 0.35f / deviceScale (g));
    g.setColour (c);
    g.strokePath (p, juce::PathStrokeType (w, joint, cap));
}

void hLine (juce::Graphics& g, float x1, float x2, float y, float logicalWidth, juce::Colour c)
{
    juce::Path p;
    p.startNewSubPath (x1, y);
    p.lineTo (x2, y);
    strokeCrisp (g, p, logicalWidth, c, juce::PathStrokeType::mitered, juce::PathStrokeType::butt);
}

void vLine (juce::Graphics& g, float x, float y1, float y2, float logicalWidth, juce::Colour c)
{
    juce::Path p;
    p.startNewSubPath (x, y1);
    p.lineTo (x, y2);
    strokeCrisp (g, p, logicalWidth, c, juce::PathStrokeType::mitered, juce::PathStrokeType::butt);
}

//==============================================================================
// Deterministic star field — tiny, crisp, sparse. No glow, no fog.
//==============================================================================
static inline juce::uint32 hashU32 (juce::uint32 x)
{
    x ^= x >> 16; x *= 0x7feb352du;
    x ^= x >> 15; x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

static inline float hashUnit (juce::uint32 x)  // 0..1
{
    return (float) (hashU32 (x) >> 8) / 16777216.0f;
}

void paintStars (juce::Graphics& g, juce::Rectangle<float> area, juce::uint32 seed,
                 float densityThreshold, int cols, int rows)
{
    if (area.getWidth() < 4.0f || area.getHeight() < 4.0f)
        return;

    const float cw = area.getWidth()  / (float) cols;
    const float ch = area.getHeight() / (float) rows;

    for (int ry = 0; ry < rows; ++ry)
    {
        for (int rx = 0; rx < cols; ++rx)
        {
            const juce::uint32 cell = hashU32 (seed + (juce::uint32) (ry * 7919 + rx * 104729));
            const float present = (float) ((cell >> 8) & 0xffff) / 65535.0f;
            if (present < densityThreshold)
                continue;

            const float jx = hashUnit (cell * 2654435761u + 11u);
            const float jy = hashUnit (cell * 40503u + 17u);
            const float x  = area.getX() + ((float) rx + 0.12f + jx * 0.76f) * cw;
            const float y  = area.getY() + ((float) ry + 0.12f + jy * 0.76f) * ch;

            const float bright = hashUnit (cell * 2246822519u + 23u);
            const float radius = 0.55f + bright * 0.75f;          // 0.55 .. 1.30 logical px
            const float alpha  = 0.26f + bright * 0.62f;          // no glow halo

            g.setColour (colour::starCore.withAlpha (alpha));
            const float d = radius * 2.0f;
            g.fillEllipse (x - radius, y - radius, d, d);

            // A handful of stars get a single-pixel brighter core so the field
            // reads as "points of light" instead of soft haze at any DPI.
            if (bright > 0.86f)
            {
                g.setColour (juce::Colours::white.withAlpha (juce::jmin (1.0f, alpha + 0.22f)));
                g.fillEllipse (x - 0.5f, y - 0.5f, 1.0f, 1.0f);
            }
        }
    }
}

//==============================================================================
// Micro-grain — a calm, deep-black brushed finish.
//
// 009 revision: the previous 008 texture filled every physical scanline across
// the whole width with white alpha 0.030..0.085 and added nine broad luminance
// bands. Measured on the 008 render that read as visible horizontal stripes.
// This version uses only short fibres, each a few logical pixels long, and
// keeps the luminance contribution strictly inside the budget:
//
//     bright fibre alpha  <= 0.008     dark fibre alpha  <= 0.015
//
// Fibre alpha is constant per strand (never scaled up), and the broad banding
// is gone entirely. The result is a flat obsidian plate with a faint brushed
// tooth instead of a stripe pattern.
//==============================================================================
namespace
{
    struct GrainCache
    {
        std::map<std::pair<int, juce::uint32>, juce::Image> tiles;
        juce::Image& get (int pixels, juce::uint32 seed)
        {
            const auto key = std::make_pair (pixels, seed);
            if (tiles.size() >= 16 && tiles.find (key) == tiles.end())
                tiles.clear();
            return tiles[key];
        }
    };

    GrainCache& grainCache()
    {
        static GrainCache cache;
        return cache;
    }

    // Base per-strand alpha. These are deliberately below the ceilings in
    // GrainLimits because overlapping strands would otherwise compound: at
    // 0.008 per strand a triple overlap reaches 0.024, which is exactly the
    // striping the 008 build showed. The clamp below makes the ceiling exact.
    constexpr float kBrightBase = 0.0070f;
    constexpr float kDarkBase   = 0.0130f;
    constexpr float kVeilBase   = 0.0035f;

    constexpr int kTileLogical = 128;   // larger tile -> no visible repeat period

    /** Built twice: once with a small base alpha, then re-stamped with the
        clamped result so the ceiling holds for every individual pixel. Alpha
        only, so stamping a black pixel with the clamped alpha is exact. */
    struct SignedBitmap
    {
        int w = 0, h = 0;
        std::vector<float> v;   // negative = dark, positive = bright
        void init (int width, int height) { w = width; h = height; v.assign ((size_t) w * h, 0.0f); }
    };

    void stamp (const juce::Image& src, SignedBitmap& acc)
    {
        juce::Image::BitmapData data (src, juce::Image::BitmapData::readOnly);
        for (int y = 0; y < acc.h; ++y)
            for (int x = 0; x < acc.w; ++x)
            {
                const auto p = data.getPixelColour (x, y);
                const float a = p.getFloatAlpha();
                if (a <= 0.0000005f)
                    continue;
                acc.v[(size_t) y * (size_t) acc.w + (size_t) x] += (p.getRed() > 127 ? a : -a);
            }
    }

    juce::Image buildGrainTile (float scale, juce::uint32 seed)
    {
        const float s = juce::jlimit (0.5f, 6.0f, scale);
        const int size = juce::roundToInt (kTileLogical * s);
        juce::Image img (juce::Image::ARGB, size, size, true);
        juce::Graphics g (img);

        const float px = (float) s;   // one logical pixel in physical pixels

        // ── Short fibres: horizontal strands of 2..7 logical px ──────────
        const int fibres = (kTileLogical * kTileLogical) / 8;
        for (int i = 0; i < fibres; ++i)
        {
            const juce::uint32 h  = hashU32 (seed + (juce::uint32) i * 2654435761u + 17u);
            const juce::uint32 h2 = hashU32 (h ^ 0x9e3779b9u);

            const float fx = (float) ((h  >>  8) & 0xfff) / 4095.0f;
            const float fy = (float) ((h  >> 20) & 0xfff) / 4095.0f;
            const float lenT = (float) ((h2 >>  6) & 0x1ff) / 511.0f;
            const float mag  = (float) ((h2 >> 18) & 0x1ff) / 511.0f;
            const bool  bright = ((h2 >> 3) & 1u) != 0u;

            const float length = (2.0f + lenT * 5.0f) * px;          // 2..7 logical px
            const float alpha  = (bright ? kBrightBase : kDarkBase) * (0.45f + 0.55f * mag);

            const float x = fx * (float) size;
            const float y = fy * (float) size;
            const float t = juce::jmax (px * 0.5f, px * (0.5f + 0.35f * mag));

            g.setColour ((bright ? juce::Colours::white : juce::Colours::black).withAlpha (alpha));
            g.fillRect (x, y, length, t);
            if (x + length > (float) size)                           // seamless in x
                g.fillRect (x - (float) size, y, length, t);
        }

        // ── Ultra-faint veil: breaks any residual row structure ──────────
        const int veils = (kTileLogical * kTileLogical) / 96;
        for (int i = 0; i < veils; ++i)
        {
            const juce::uint32 h = hashU32 (seed * 31u + (juce::uint32) i * 40503u + 5u);
            const float fx = (float) ((h >>  8) & 0xfff) / 4095.0f;
            const float fy = (float) ((h >> 20) & 0xfff) / 4095.0f;
            const float lm = (float) ((h >>  4) & 0x3f) / 63.0f;
            const float length = (24.0f + lm * 40.0f) * px;
            const bool bright = (h & 2u) != 0u;
            g.setColour ((bright ? juce::Colours::white : juce::Colours::black)
                             .withAlpha (kVeilBase * (0.4f + 0.6f * lm)));
            const float x = fx * (float) size;
            g.fillRect (x, fy * (float) size, length, px * 0.6f);
            if (x + length > (float) size)
                g.fillRect (x - (float) size, fy * (float) size, length, px * 0.6f);
        }

        // ── Exact per-pixel ceiling ─────────────────────────────────────
        // Overlapping strands are summed first, then clamped to the audited
        // budget and stamped back. Nothing in the tile can exceed
        // GrainLimits::brightFibre (bright) or ::darkFibre (dark).
        SignedBitmap acc;
        acc.init (size, size);
        stamp (img, acc);

        juce::Image clamped (juce::Image::ARGB, size, size, true);
        juce::Graphics cg (clamped);
        cg.setColour (juce::Colours::transparentBlack);
        cg.fillAll();
        for (int y = 0; y < size; ++y)
            for (int x = 0; x < size; ++x)
            {
                const float v = acc.v[(size_t) y * (size_t) size + (size_t) x];
                if (v > 0.0f)
                    cg.setColour (juce::Colours::white.withAlpha (juce::jmin (v, GrainLimits::brightFibre)));
                else if (v < 0.0f)
                    cg.setColour (juce::Colours::black.withAlpha (juce::jmin (-v, GrainLimits::darkFibre)));
                else
                    continue;
                cg.fillRect ((float) x, (float) y, 1.0f, 1.0f);
            }
        return clamped;
    }
}

void paintBrushedGrain (juce::Graphics& g, juce::Rectangle<float> area, juce::uint32 seed)
{
    if (area.getWidth() < 1.0f || area.getHeight() < 1.0f)
        return;

    const int tilePixels = juce::roundToInt (kTileLogical * juce::jlimit (0.5f, 6.0f, deviceScale (g)));
    const float scale = (float) tilePixels / (float) kTileLogical;
    auto& cache = grainCache();
    auto& tile = cache.get (tilePixels, seed);
    if (! tile.isValid())
        tile = buildGrainTile (scale, seed);

    if (! tile.isValid())
        return;

    g.saveState();
    g.reduceClipRegion (area.toNearestInt());
    g.setFillType (juce::FillType (tile, juce::AffineTransform::scale (1.0f / scale)));
    g.fillRect (area);
    g.restoreState();
}

bool measureGrainTile (int scale, float& maxWhiteAlpha, float& maxBlackAlpha)
{
    const int s = juce::jlimit (1, 6, scale);
    const auto tile = buildGrainTile (s, 0x9e3779b9u);
    if (! tile.isValid())
        return false;

    juce::Image::BitmapData data (tile, juce::Image::BitmapData::readOnly);
    maxWhiteAlpha = 0.0f;
    maxBlackAlpha = 0.0f;
    for (int y = 0; y < tile.getHeight(); ++y)
    {
        for (int x = 0; x < tile.getWidth(); ++x)
        {
            const auto p = data.getPixelColour (x, y);
            const float a = p.getFloatAlpha();
            if (a <= 0.000001f)
                continue;
            if (p.getRed() > 127) maxWhiteAlpha = juce::jmax (maxWhiteAlpha, a);
            else                  maxBlackAlpha = juce::jmax (maxBlackAlpha, a);
        }
    }
    return true;
}

//==============================================================================
// Layer 0 — window backdrop
//==============================================================================
void paintWindowBackdrop (juce::Graphics& g, juce::Rectangle<float> whole)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0b0c0d), whole.getCentreX(), whole.getY(),
                                             juce::Colour (0xff050506), whole.getCentreX(), whole.getBottom(),
                                             false));
    g.fillRect (whole);

    // Faint edge darkening only — no grey wash, no coloured bloom.
    juce::ColourGradient vig (juce::Colours::transparentBlack, whole.getCentreX(), whole.getCentreY(),
                              juce::Colours::black.withAlpha (0.40f), whole.getX(), whole.getY(), true);
    g.setGradientFill (vig);
    g.fillRect (whole);
}

//==============================================================================
// Layer 1 — chassis plate (metal body). Bottom strip is drawn separately.
//==============================================================================
void paintChassisPlate (juce::Graphics& g, juce::Rectangle<float> plate, bool bottomStrip)
{
    // Slight top-lit vertical gradient: this is what makes it read as a
    // physical chassis rather than a flat card.
    juce::Colour top    = bottomStrip ? juce::Colour (0xff151617) : colour::chassisTop;
    juce::Colour bottom = bottomStrip ? juce::Colour (0xff0a0b0c) : colour::chassisBottom;
    juce::ColourGradient face (top, plate.getCentreX(), plate.getY(),
                               bottom, plate.getCentreX(), plate.getBottom(), false);
    face.addColour (0.18, top.interpolatedWith (bottom, 0.38f));
    face.addColour (0.70, top.interpolatedWith (bottom, 0.80f));
    g.setGradientFill (face);
    g.fillRect (plate);

    // Micro-grain only. The 008 nine-band luminance banding was removed here:
    // it read as broad horizontal stripes once the plate was that calm.
    paintBrushedGrain (g, plate, bottomStrip ? 0x51ed2701u : 0x9e3779b9u);

    // Edge darkening at the very border of the plate.
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.35f), plate.getX(), plate.getCentreY(),
                                             juce::Colours::transparentBlack, plate.getX() + 12.0f, plate.getCentreY(), false));
    g.fillRect (plate.withWidth (12.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.35f), plate.getRight(), plate.getCentreY(),
                                             juce::Colours::transparentBlack, plate.getRight() - 12.0f, plate.getCentreY(), false));
    g.fillRect (plate.withTrimmedLeft (plate.getWidth() - 12.0f));
    // A tight underside contact edge gives depth without a blurred shadow.
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack,
                                             plate.getCentreX(), plate.getBottom() - 4.0f,
                                             juce::Colours::black.withAlpha (0.32f),
                                             plate.getCentreX(), plate.getBottom(), false));
    g.fillRect (plate.withTop (plate.getBottom() - 4.0f));
}

//==============================================================================
// Layer 2 — machined bezel: chamfer band + bright catch-light + drop shadow
//==============================================================================
void paintBezel (juce::Graphics& g, juce::Rectangle<float> r)
{
    // A narrow milled rim leaves the black face visible. All catches follow
    // one overhead light; no broad grey frame or soft shadow texture.
    juce::Path rim;
    rim.addRoundedRectangle (r, 5.0f);
    rim.addRoundedRectangle (r.reduced (3.0f), 2.0f);
    rim.setUsingNonZeroWinding (false);
    juce::ColourGradient metal (juce::Colour (0xff3a3f43), r.getCentreX(), r.getY(),
                                juce::Colour (0xff101316), r.getCentreX(), r.getBottom(), false);
    metal.addColour (0.20, juce::Colour (0xff25292c));
    g.setGradientFill (metal);
    g.fillPath (rim);
    g.setColour (juce::Colour (0xff07090a));
    g.drawRoundedRectangle (r.reduced (3.5f), 2.0f, 1.0f);
    g.setColour (colour::bezelHighlight.withAlpha (0.20f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.5f, 0.8f);
    hLine (g, r.getX() + 6.0f, r.getRight() - 6.0f, r.getY() + 0.6f, 0.8f,
           juce::Colour (0xffd5dce1).withAlpha (0.34f));
    vLine (g, r.getX() + 0.6f, r.getY() + 6.0f, r.getBottom() - 6.0f, 0.8f,
           juce::Colour (0xffc1cbd3).withAlpha (0.12f));
    vLine (g, r.getRight() - 0.6f, r.getY() + 6.0f, r.getBottom() - 6.0f, 0.8f,
           juce::Colour (0xffc1cbd3).withAlpha (0.10f));
    hLine (g, r.getX() + 6.0f, r.getRight() - 6.0f, r.getBottom() - 0.8f, 0.8f,
           juce::Colours::black.withAlpha (0.8f));
}

void paintInsetSeam (juce::Graphics& g, juce::Rectangle<float> r)
{
    if (r.getWidth() < 8.0f || r.getHeight() < 8.0f)
        return;
    // A shallow recessed panel edge: one dark seam with a thin light lip just
    // inside it. No heavy ring, so the glass and the keys stay dominant.
    strokeCrisp (g, [&] { juce::Path p; p.addRoundedRectangle (r, 3.0f); return p; }(),
                 1.0f, juce::Colours::black.withAlpha (0.55f));
    strokeCrisp (g, [&] { juce::Path p; p.addRoundedRectangle (r.reduced (1.2f), 3.0f); return p; }(),
                 0.8f, juce::Colours::white.withAlpha (0.045f));
    hLine (g, r.getX() + 4.0f, r.getRight() - 4.0f, r.getY() + 1.0f, 0.8f,
           juce::Colours::black.withAlpha (0.45f));
    hLine (g, r.getX() + 4.0f, r.getRight() - 4.0f, r.getBottom() - 0.6f, 0.7f,
           juce::Colours::white.withAlpha (0.075f));
}

//==============================================================================
// Layer 3 — glass frequency screen (stars + grid are painted by the editor
//           on top of this, because they depend on the live FIR response)
//==============================================================================
void paintGlass (juce::Graphics& g, juce::Rectangle<float> r)
{
    // Three tightly bounded layers form a metal lip, black gasket and glass.
    // Their geometry is re-evaluated at the actual render scale.
    g.setColour (juce::Colours::black.withAlpha (0.95f));
    g.fillRoundedRectangle (r.expanded (1.0f), 4.5f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff30363b), r.getCentreX(), r.getY(),
                                             juce::Colour (0xff101418), r.getCentreX(), r.getBottom(), false));
    g.drawRoundedRectangle (r.expanded (0.5f), 4.0f, 1.0f);
    juce::ColourGradient glass (juce::Colour (0xff0a0d10), r.getX(), r.getY(),
                                juce::Colour (0xff030506), r.getRight(), r.getBottom(), false);
    glass.addColour (0.48, juce::Colour (0xff06090b));
    g.setGradientFill (glass);
    g.fillRoundedRectangle (r.reduced (1.0f), 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.88f));
    g.drawRoundedRectangle (r.reduced (2.0f), 2.0f, 1.0f);
    hLine (g, r.getX() + 5.0f, r.getRight() - 5.0f, r.getY() + 3.6f, 1.0f,
           juce::Colours::black.withAlpha (0.45f));
    hLine (g, r.getX() + 5.0f, r.getRight() - 5.0f, r.getY() + 2.7f, 0.65f,
           juce::Colour (0xffb6c6d2).withAlpha (0.09f));
    hLine (g, r.getX() + 5.0f, r.getRight() - 5.0f, r.getBottom() - 0.6f, 0.8f,
           juce::Colour (0xffa4afb8).withAlpha (0.16f));
}

//==============================================================================
// Layer 4 — physical key
//==============================================================================
void paintKey (juce::Graphics& g, juce::Rectangle<float> r, bool active, bool over, bool down)
{
    const float radius = 2.5f;
    const auto face = r.reduced (1.0f).translated (0.0f, down ? 1.0f : 0.0f);
    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.5f), radius + 1.0f);
    hLine (g, r.getX() + 3.0f, r.getRight() - 3.0f, r.getBottom() - 0.5f, 1.0f,
           juce::Colours::black.withAlpha (down ? 0.35f : 0.72f));
    const auto top = active ? juce::Colour (0xff414549) : juce::Colour (0xff303235);
    const auto bottom = active ? juce::Colour (0xff25282c) : juce::Colour (0xff17181b);
    juce::ColourGradient metal (down ? bottom : top, face.getCentreX(), face.getY(),
                                down ? top : bottom, face.getCentreX(), face.getBottom(), false);
    metal.addColour (0.25, active ? juce::Colour (0xff34383c) : juce::Colour (0xff292b2e));
    metal.addColour (0.70, active ? juce::Colour (0xff2b2e32) : juce::Colour (0xff1d1f22));
    g.setGradientFill (metal);
    g.fillRoundedRectangle (face, radius);
    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path clip; clip.addRoundedRectangle (face, radius);
        g.reduceClipRegion (clip);
        paintBrushedGrain (g, face, 0x85ebca6bu);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), face.getX(), 0,
                                                juce::Colours::transparentWhite, face.getX() + 2, 0, false));
        g.fillRect (face.withWidth (2));
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.045f), face.getRight(), 0,
                                                juce::Colours::transparentWhite, face.getRight() - 2, 0, false));
        g.fillRect (face.withTrimmedLeft (face.getWidth() - 2));
    }
    g.setColour (juce::Colours::black.withAlpha (0.90f));
    g.drawRoundedRectangle (face, radius, 0.8f);
    hLine (g, face.getX() + 4, face.getRight() - 4, face.getY() + 0.5f, 0.75f,
           juce::Colours::white.withAlpha (down ? 0.08f : over ? 0.34f : 0.23f));
    hLine (g, face.getX() + 4, face.getRight() - 4, face.getBottom() - 0.5f, 0.75f,
           juce::Colour (0xff6d7277).withAlpha (down ? 0.35f : 0.12f));
    if (over || active)
    {
        g.setColour (colour::hoverEdge.withAlpha (over ? 0.35f : 0.20f));
        g.drawRoundedRectangle (face.reduced (0.5f), radius, 0.7f);
    }
}

} // namespace hm::skin
