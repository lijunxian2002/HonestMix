#include "UI/SealComponent.h"
#include "Core/Design.h"

void SealComponent::setDotOn (bool on)
{
    if (dotOn_ != on) { dotOn_ = on; repaint(); }
}

void SealComponent::paint (juce::Graphics& g)
{
    const bool hov = isMouseOver();

    // 纯文字印章（无方印底框）—— 与左侧 BPM 文字形成对称
    g.setFont (hm::kaiTi ((float) getHeight() * 0.62f));
    g.setColour (hov ? hm::accentWarm() : hm::gold().withAlpha (0.75f));
    g.drawText (juce::String::fromUTF8 (u8"诚"), getLocalBounds(), juce::Justification::centred);

    if (dotOn_)
    {
        g.setColour (hm::accentWarm().withAlpha (0.7f));
        g.fillEllipse ((float) getWidth() - 14.0f, 4.0f, 6.0f, 6.0f);
    }
}

void SealComponent::mouseDown (const juce::MouseEvent&)
{
    if (onClick) onClick();
}
