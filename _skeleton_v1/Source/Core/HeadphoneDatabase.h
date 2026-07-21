#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * 耳机校正曲线数据库
 *
 * 职责：
 *   - 管理各耳机型号对应的 EQ 校正曲线数据
 *   - 从内置资源或外部文件加载曲线（JSON / Binary）
 *   - 根据耳机型号 ID 返回对应的 Filter 系数
 *
 * 每条曲线由一系列频率-增益点定义（频响曲线），
 * CorrectionProcessor 使用这些数据构建 IIR/FIR 滤波器。
 *
 * 数据格式（内部 JSON）：
 *   {
 *     "id": "ath_m50x",
 *     "name": "Audio-Technica ATH-M50X",
 *     "target": "Harman OE 2018",
 *     "description": "...",
 *     "frequencies": [20, 25, 31, ...],
 *     "gainsDb":    [2.1, 1.8, 1.5, ...]
 *   }
 */
class HeadphoneDatabase
{
public:
    //==============================================================================
    /** 一条校正曲线 */
    struct Curve
    {
        juce::String id;                   ///< 唯一标识，与 Settings::HeadphoneEntry.curveId 对应
        juce::String name;                 ///< 显示名称
        juce::String target;               ///< 目标曲线名称 (Harman OE / diffuse-field …)
        juce::String description;          ///< 描述（来源、测量条件等）
        juce::Array<float> frequencies;    ///< 频率点 (Hz)
        juce::Array<float> gainsDb;        ///< 每个频率点的增益值 (dB)
    };

    //==============================================================================
    HeadphoneDatabase();

    // ——— 查询 ———

    /** 返回所有可用曲线的 ID 列表 */
    juce::StringArray getAvailableIds() const;

    /** 根据 curveId 获取曲线（如未找到返回 nullptr） */
    const Curve* getCurveById(const juce::String& curveId) const;

    /** 返回某个曲线是否已被加载 */
    bool hasCurve(const juce::String& curveId) const;

    // ——— 加载 ———

    /** 从 JSON 字符串加载一条曲线 */
    bool loadFromJson(const juce::String& curveId, const juce::String& jsonString);

    /** 从 File 加载一条曲线 */
    bool loadFromFile(const juce::File& file);

    /** 从项目 BinaryResources 加载所有内置曲线 */
    int loadBuiltInCurves();

    // ——— 管理 ———

    /** 返回曲线数量 */
    int getNumCurves() const noexcept { return curves.size(); }

    /** 清空所有曲线 */
    void clear();

private:
    //==============================================================================
    juce::OwnedArray<Curve> curves;

    /** 解析单条 JSON → Curve */
    Curve* parseCurve(const juce::var& json);

    /** 根据曲线计算最大 / 最小频率范围 */
    static juce::Range<float> getFreqRange(const Curve& curve);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeadphoneDatabase)
};
