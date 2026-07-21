#include "HeadphoneDatabase.h"

//==============================================================================
// HeadphoneDatabase — 纯接口骨架
//
// 所有内置曲线数据已被剥离。
// 工程师后续通过以下方式注入数据：
//   1. loadFromJson(curveId, jsonString) — 从 JSON 字符串加载单条曲线
//   2. loadFromFile(file)                — 从外部 JSON 文件加载
//   3. 替换 loadBuiltInCurves()           — 从 BinaryResources 加载
//==============================================================================

HeadphoneDatabase::HeadphoneDatabase() {}

//==============================================================================
// ——— 查询 ———

juce::StringArray HeadphoneDatabase::getAvailableIds() const
{
    juce::StringArray ids;
    for (auto* c : curves)
        ids.add(c->id);
    return ids;
}

const HeadphoneDatabase::Curve* HeadphoneDatabase::getCurveById(const juce::String& curveId) const
{
    for (auto* c : curves)
        if (c->id == curveId)
            return c;
    return nullptr;
}

bool HeadphoneDatabase::hasCurve(const juce::String& curveId) const
{
    return getCurveById(curveId) != nullptr;
}

//==============================================================================
// ——— 加载 ———

bool HeadphoneDatabase::loadFromJson(const juce::String& curveId, const juce::String& jsonString)
{
    auto json = juce::JSON::parse(jsonString);

    if (!json.isObject())
        return false;

    auto* parsed = parseCurve(json);
    if (parsed == nullptr)
        return false;

    // 如果同 ID 已存在则替换
    for (int i = 0; i < curves.size(); ++i)
    {
        if (curves[i]->id == curveId)
        {
            curves.remove(i);
            curves.insert(i, parsed);
            return true;
        }
    }

    curves.add(parsed);
    return true;
}

bool HeadphoneDatabase::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    auto jsonString = file.loadFileAsString();
    auto id = file.getFileNameWithoutExtension();
    return loadFromJson(id, jsonString);
}

int HeadphoneDatabase::loadBuiltInCurves()
{
    // 无内置曲线 — 工程师后续通过 BinaryResources 或 JSON 加载
    return 0;
}

//==============================================================================
// ——— 管理 ———

void HeadphoneDatabase::clear()
{
    curves.clear();
}

//==============================================================================
// ——— 内部方法 ———

HeadphoneDatabase::Curve* HeadphoneDatabase::parseCurve(const juce::var& json)
{
    auto obj = json.getDynamicObject();
    if (obj == nullptr) return nullptr;

    auto* curve = new Curve();
    curve->id          = obj->getProperty("id");
    curve->name        = obj->getProperty("name");
    curve->target      = obj->getProperty("target");
    curve->description = obj->getProperty("description");

    // 解析频率数组
    auto freqs = obj->getProperty("frequencies");
    if (auto* freqArray = freqs.getArray())
    {
        for (auto& f : *freqArray)
            curve->frequencies.add(static_cast<float>(f));
    }

    // 解析增益数组
    auto gains = obj->getProperty("gainsDb");
    if (auto* gainArray = gains.getArray())
    {
        for (auto& g : *gainArray)
            curve->gainsDb.add(static_cast<float>(g));
    }

    // 验证数据完整性
    if (curve->frequencies.size() != curve->gainsDb.size() || curve->frequencies.isEmpty())
    {
        delete curve;
        return nullptr;
    }

    return curve;
}
