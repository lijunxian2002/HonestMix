#pragma once
//==============================================================================
/**
 * HONESTMIX Headphone — "Obsidian Stars" layered skin (UI only).
 *
 * The whole surface is drawn as real JUCE vector work at the current device
 * scale: base plate -> glass -> metal bezel -> fader slot/cap -> keys.
 * Nothing here is a stretched bitmap of the design reference. The only cached
 * raster is procedural micro-grain, regenerated at the drawing scale with
 * exact native tiles at 100/125/150/200 percent. No reference bitmap is used.
 */
#include <juce_gui_basics/juce_gui_basics.h>

namespace hm::skin
{
    // ── Logical design size. Everything below is in these units. ──────────
    constexpr int kDesignW = 1000;
    constexpr int kDesignH = 480;

    // ── Chassis panels (logical coordinates) ─────────────────────────────
    // topPlate  : 8,8,984,432   metal body + bezel + glass + fader + 2 keys
    // strip     : 440..480      quiet status rail
    constexpr int kTopPlateBottom   = 440;
    constexpr int kBrandBarHeight   = 40;
    constexpr int kStripTopY        = kTopPlateBottom;
    constexpr int kStripHeight      = kDesignH - kTopPlateBottom;
    constexpr int kBezelOuter       = 3;   // narrow machined edge band
    constexpr int kBezelInner       = 4;   // inner panel seam

    // Hero geometry. Preserves the accepted shape: wide glass on the left, a
    // vertical mechanical fader on the right, two physical keys along the
    // bottom of the chassis.
    inline juce::Rectangle<int> glassPanelBounds()  { return { 26, 62, 720, 322 }; }
    inline juce::Rectangle<int> faderColumnBounds() { return { 766, 62, 208, 373 }; }
    inline juce::Rectangle<int> keyBounds (int i)
    {
        return i == 0 ? juce::Rectangle<int> { 34, 391, 336, 46 }
                      : juce::Rectangle<int> { 394, 391, 336, 46 };
    }
    /** Headphone model / change-headphone entry, inside the top of the glass. */
    inline juce::Rectangle<int> titleChipBounds() { return { 42, 70, 688, 30 }; }

    // ── Palette: single cool-white metal, deep obsidian glass. ───────────
    // Neutral-cool greys only. No blue cast, no fog, no coloured glow.
    namespace colour
    {
        const juce::Colour windowBackground  { 0xff060607 };
        const juce::Colour chassisTop        { 0xff1d1f21 };
        const juce::Colour chassisBottom     { 0xff101214 };
        const juce::Colour plateFace         { 0xff161718 };
        const juce::Colour plateFaceDeep     { 0xff0c0d0e };
        const juce::Colour bezelHighlight    { 0xff8e9095 };
        const juce::Colour bezelShadow       { 0xff000000 };
        const juce::Colour seamLine          { 0xff000000 };
        const juce::Colour glassTop          { 0xff08090a };
        const juce::Colour glassBottom       { 0xff030304 };
        const juce::Colour glassEdge         { 0xff26282c };
        const juce::Colour gridMajor         { 0xff282b2f };
        const juce::Colour gridMinor         { 0xff15171a };
        const juce::Colour curve             { 0xffe6e9ec };
        const juce::Colour curveUnder        { 0x4d1d2024 };
        const juce::Colour textPrimary       { 0xffdfe2e5 };
        const juce::Colour textSecondary     { 0xff9a9da2 };
        const juce::Colour textDim           { 0xff6b6e73 };
        const juce::Colour starCore          { 0xfff0f3f6 };
        const juce::Colour keyTop            { 0xff303235 };
        const juce::Colour keyBottom         { 0xff151618 };
        const juce::Colour keyActiveTop      { 0xff474b50 };
        const juce::Colour keyActiveBottom   { 0xff24272a };
        const juce::Colour slotInner         { 0xff030304 };
        const juce::Colour capTop            { 0xff9aa0a6 };   // brushed steel highlight
        const juce::Colour capMid            { 0xff4b5055 };
        const juce::Colour capBottom         { 0xff1a1c1e };
        const juce::Colour focusRing         { 0xffdfe2e5 };
        const juce::Colour warn              { 0xffcfa184 };
        const juce::Colour hoverEdge         { 0xffb9bec4 };
    }

    // ── Native vector drawing helpers (all resolution independent) ───────
    juce::Font   uiFont (float logicalHeight, bool bold = false);

    void drawText (juce::Graphics&, const juce::String&, juce::Rectangle<int>,
                   float logicalHeight, juce::Colour,
                   juce::Justification = juce::Justification::centredLeft,
                   bool bold = false);

    /** Draws a crisp stroke whose width is given in LOGICAL pixels. */
    void strokeCrisp (juce::Graphics&, const juce::Path&, float logicalWidth,
                      juce::Colour,
                      juce::PathStrokeType::JointStyle = juce::PathStrokeType::mitered,
                      juce::PathStrokeType::EndCapStyle = juce::PathStrokeType::butt);

    void hLine (juce::Graphics&, float x1, float x2, float y, float logicalWidth, juce::Colour);
    void vLine (juce::Graphics&, float x, float y1, float y2, float logicalWidth, juce::Colour);

    /** Deterministic star field inside a component. Sparsity is controlled by
        densityThreshold in 0..1 (higher = fewer stars). */
    void paintStars (juce::Graphics&, juce::Rectangle<float>, juce::uint32 seed,
                     float densityThreshold = 0.45f, int cols = 20, int rows = 9);

    // ── Layered plate / glass / metal (each drawn separately) ────────────
    void paintWindowBackdrop (juce::Graphics&, juce::Rectangle<float> whole);
    void paintChassisPlate   (juce::Graphics&, juce::Rectangle<float> plate, bool bottomStrip);
    void paintBezel          (juce::Graphics&, juce::Rectangle<float> r);
    void paintInsetSeam     (juce::Graphics&, juce::Rectangle<float> r);
    void paintGlass          (juce::Graphics&, juce::Rectangle<float> r);
    void paintKey            (juce::Graphics&, juce::Rectangle<float> r, bool active, bool over, bool down);

    /** Procedural micro-grain, regenerated at the current drawing scale.
        Exact native tiles at quarter scales; a bounded material cache. */
    void paintBrushedGrain (juce::Graphics&, juce::Rectangle<float>, juce::uint32 seed);

    /** Auditable alpha ceilings actually used by the grain tile (009 fix).
        The clamp target is chosen so that even after 8-bit quantisation the
        measured alpha can never exceed the stated budget: a round-up of at most
        1/255 is possible, so a target of 0.0129 stays below the 0.015 budget. */
    struct GrainLimits
    {
        static constexpr float brightFibre = 0.0076f;   // budget <= 0.008
        static constexpr float darkFibre   = 0.0129f;   // budget <= 0.015
        static constexpr float veil        = 0.0035f;
        static constexpr int   tileLogical = 128;

        // Stated budget from the 009 request, checked against the measurement.
        static constexpr float budgetBright = 0.008f;
        static constexpr float budgetDark   = 0.015f;
    };
    /** Fills `out` with the measured per-channel extrema of the generated tile
        at `scale`; used by the evidence harness. Returns false if unavailable. */
    bool measureGrainTile (int scale, float& maxWhiteAlpha, float& maxBlackAlpha);

    /** Current logical -> device scale factor of the drawing pass. Set by the
        editor for both the on-screen transform and the offline renders, so that
        every hairline stays exactly one physical pixel wide. */
    void  setDrawingScale (float scale);
    float drawingScale();
    float deviceScale (juce::Graphics&);
}
