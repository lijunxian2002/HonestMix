#include "Core/HeadphoneDatabase.h"

// 全工程唯一包含 25MB FIR 数据表的翻译单元
#include "dsp/fir_data.h"   // kProfiles, NUM_PROFILES, HeadphoneInfo

HeadphoneDatabase::HeadphoneDatabase()
    : table_ (kProfiles), count_ (NUM_PROFILES)
{
}

//==============================================================================
// ——— 耳机档案 ———

int HeadphoneDatabase::getNumProfiles() const noexcept
{
    return count_;
}

const char* HeadphoneDatabase::getProfileName (int index) const noexcept
{
    if (index < 0 || index >= count_)
        return "Unknown";
    return table_[index].name;
}

const float* HeadphoneDatabase::getProfileFIR (int index) const noexcept
{
    if (index < 0 || index >= count_)
        return nullptr;
    return table_[index].fir;
}

int HeadphoneDatabase::findProfileByName (const juce::String& name) const noexcept
{
    for (int i = 0; i < count_; ++i)
        if (juce::String (table_[i].name) == name)
            return i;
    return -1;
}

void HeadphoneDatabase::search (const juce::String& filter, std::vector<int>& out,
                                int maxResults) const
{
    out.clear();
    const juce::String f = filter.toLowerCase();
    if (f.isEmpty())
        return;

    for (int i = 0; i < count_; ++i)
    {
        if (juce::String (table_[i].name).toLowerCase().contains (f))
        {
            out.push_back (i);
            if (maxResults > 0 && (int) out.size() >= maxResults)
                break;
        }
    }
}

juce::String HeadphoneDatabase::shortName (const juce::String& full)
{
    const int sp = full.indexOfChar (' ');
    return sp > 0 ? full.substring (sp + 1) : full;
}

juce::String HeadphoneDatabase::brandOf (const juce::String& full)
{
    const int sp = full.indexOfChar (' ');
    return sp > 0 ? full.substring (0, sp) : full;
}

//==============================================================================
// ——— 静态表（源自数据中心 HeadphoneBrowser::curveNames_ / ifNames_ / 预设）———

const juce::StringArray& HeadphoneDatabase::getCurveNames()
{
    static const juce::StringArray curves {
        "Harman OE", "Diffuse Field", "Free Field", "Harman IE"
    };
    return curves;
}

const juce::StringArray& HeadphoneDatabase::getInterfaceNames()
{
    static const juce::StringArray ifs {
        "RME Babyface Pro", "RME UCX II", "Focusrite Scarlett", "UA Apollo Twin",
        "SSL 2+", "Audient iD14", "Motu M2", juce::String::fromUTF8 (u8"集成 / 其它")
    };
    return ifs;
}

const juce::StringArray& HeadphoneDatabase::getPopularPresets()
{
    // 引导页 8 副热门耳机（全名，须与 kProfiles 中条目完全一致）
    static const juce::StringArray presets {
        "Audio-Technica ATH-M50x", "Beyerdynamic DT 770 Pro",
        "Sennheiser HD 600",       "Sennheiser HD 650",
        "Beyerdynamic DT 990 Pro", "Sony MDR-7506",
        "Sennheiser HD 560S",      "AKG K701"
    };
    return presets;
}
