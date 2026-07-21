#include "TimeReferenceTable.h"
#include "../LookAndFeel.h"
#include "../../Core/Settings.h"

//==============================================================================
TimeReferenceTable::TimeReferenceTable(Type type, const juce::String& title)
    : type(type), tableTitle(title)
{
    regenerateRows();
}

//==============================================================================
void TimeReferenceTable::setBPM(int bpm)
{
    currentBPM = juce::jlimit(20, 300, bpm);
    calculator.setBPM(currentBPM);
    regenerateRows();
    repaint();
}

//==============================================================================
void TimeReferenceTable::regenerateRows()
{
    rows.clear();

    switch (type)
    {
    case Type::PreDelay:
    {
        const auto& labels = Settings::getPreDelayLabels();
        for (const auto& [ms, desc] : labels)
            rows.add({ juce::String(ms) + " ms", (double)ms, desc, {} });
        break;
    }

    case Type::Reverb:
    {
        double roomMs  = calculator.getRoomReverbMs();
        double plateMs = calculator.getPlateReverbMs();
        double hallMs  = calculator.getHallReverbMs();

        rows.add({ "ROOM 房间", roomMs, "干练 / 自然", {} });
        rows.add({ "PLATE 板式", plateMs, "明亮 / 饱满", {} });
        rows.add({ "HALL 大厅", hallMs, "辽阔 / 宏大", {} });
        break;
    }

    case Type::Release:
    {
        double fastMs   = calculator.getFastReleaseMs();
        double mediumMs = calculator.getMediumReleaseMs();
        double slowMs   = calculator.getSlowReleaseMs();

        rows.add({ "快速", fastMs, "灵活 / 紧实", {} });
        rows.add({ "中速", mediumMs, "自然 / 流畅", {} });
        rows.add({ "慢速", slowMs, "平滑 / 沉稳", {} });
        break;
    }

    case Type::Delay:
    {
        const auto& notes = Settings::getDelayNoteRatios();
        for (const auto& [note, ratio] : notes)
        {
            double ms = calculator.getDelayMs(ratio);
            rows.add({ note, ms, {}, {} });
        }
        break;
    }
    }
}

//==============================================================================
void TimeReferenceTable::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // 标题
    auto titleBounds = bounds.removeFromTop(18);
    g.setColour(HonestMixLookAndFeel::textMuted);
    g.setFont(juce::FontOptions(9.0f));
    g.drawText(tableTitle, titleBounds.toFloat(),
               juce::Justification::centredLeft, true);

    // 表格
    g.setFont(juce::FontOptions(8.0f));
    for (int i = 0; i < rows.size(); ++i)
    {
        auto& row = rows[i];
        int y = titleBounds.getBottom() + 4 + i * 20;

        // 标签
        g.setColour(HonestMixLookAndFeel::textSubtle);
        g.drawText(row.label, 0, y, bounds.getWidth() / 2, 18,
                   juce::Justification::centredLeft, true);

        // 数值
        g.setColour(HonestMixLookAndFeel::textMuted);
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(BPMCalculator::formatMs(row.valueMs, 0),
                   bounds.getWidth() / 2, y, bounds.getWidth() / 4, 18,
                   juce::Justification::centredLeft, true);

        // 描述
        if (row.description.isNotEmpty())
        {
            g.setColour(HonestMixLookAndFeel::textSubtle);
            g.setFont(juce::FontOptions(7.0f));
            g.drawText(row.description,
                       bounds.getWidth() * 3 / 4, y, bounds.getWidth() / 4, 18,
                       juce::Justification::centredLeft, true);
        }
    }
}

void TimeReferenceTable::resized()
{
    // 自动适应父级宽度
}
