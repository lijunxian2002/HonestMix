#include "ValueTreeHelpers.h"

//==============================================================================
// ——— 读取 ———

int ValueTreeHelpers::getInt(const juce::ValueTree& tree,
                             const juce::Identifier& property,
                             int defaultValue)
{
    return tree.getProperty(property, defaultValue);
}

float ValueTreeHelpers::getFloat(const juce::ValueTree& tree,
                                 const juce::Identifier& property,
                                 float defaultValue)
{
    return tree.getProperty(property, defaultValue);
}

double ValueTreeHelpers::getDouble(const juce::ValueTree& tree,
                                   const juce::Identifier& property,
                                   double defaultValue)
{
    return tree.getProperty(property, defaultValue);
}

bool ValueTreeHelpers::getBool(const juce::ValueTree& tree,
                               const juce::Identifier& property,
                               bool defaultValue)
{
    return static_cast<bool>(tree.getProperty(property, defaultValue));
}

juce::String ValueTreeHelpers::getString(const juce::ValueTree& tree,
                                         const juce::Identifier& property,
                                         const juce::String& defaultValue)
{
    return tree.getProperty(property, defaultValue).toString();
}

//==============================================================================
// ——— 写入 ———

void ValueTreeHelpers::setInt(juce::ValueTree& tree,
                              const juce::Identifier& property,
                              int value)
{
    tree.setProperty(property, value, nullptr);
}

void ValueTreeHelpers::setFloat(juce::ValueTree& tree,
                                const juce::Identifier& property,
                                float value)
{
    tree.setProperty(property, value, nullptr);
}

void ValueTreeHelpers::setDouble(juce::ValueTree& tree,
                                 const juce::Identifier& property,
                                 double value)
{
    tree.setProperty(property, value, nullptr);
}

void ValueTreeHelpers::setBool(juce::ValueTree& tree,
                               const juce::Identifier& property,
                               bool value)
{
    tree.setProperty(property, value, nullptr);
}

void ValueTreeHelpers::setString(juce::ValueTree& tree,
                                 const juce::Identifier& property,
                                 const juce::String& value)
{
    tree.setProperty(property, value, nullptr);
}

//==============================================================================
// ——— 子结点查找 ———

juce::ValueTree ValueTreeHelpers::getOrCreateChild(juce::ValueTree& parent,
                                                    const juce::Identifier& childTag,
                                                    int index)
{
    auto child = parent.getChildWithName(childTag);

    if (child.isValid())
        return child;

    if (index < 0)
    {
        child = juce::ValueTree(childTag);
        parent.appendChild(child, nullptr);
    }
    else
    {
        child = juce::ValueTree(childTag);
        parent.appendChild(child, nullptr);
    }

    return child;
}

//==============================================================================
juce::ValueTree ValueTreeHelpers::deepCopy(const juce::ValueTree& source)
{
    return source.createCopy();
}
