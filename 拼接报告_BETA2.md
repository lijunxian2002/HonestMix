# HonestMix BETA2 — 拼接报告

> 拼接日期：2026-07-21 · 拼接人：ZCode（最终把关）
> 产物：`BETA2.vst3`（v0.4.0，11MB）→ 已部署 `C:\Program Files\Common Files\VST3\`
> 数据中心：`C:\Users\Administrator\source\repos\HonestMix`（仓库A）
> 骨架中心：`HonestMix_源码全集.md`（文件2，存档于 `_skeleton_v1/`）

---

## 一、拼接总原则

**骨架定结构，数据定行为。**
文件2（骨架）提供模块边界与文件布局；文件1（数据中心）的已调优实现填充每个槽位。
骨架中被数据中心明确否决的部分（IIR 十带参均衡、APVTS 自动化、空数据桩）不进入 BETA2。

## 二、模块映射表（症状 → 文件，修 BUG 查这里）

| 症状领域 | 文件 | 来源 |
|:---|:---|:---|
| 颜色/字体/布局比例 | `Source/Core/Design.h` | 数据中心 hm:: 原样 |
| 耳机/曲线/声卡数据、搜索 | `Source/Core/HeadphoneDatabase.*` | 骨架槽位 × kProfiles 绑定 |
| 持久化字段（度/耳机/引导/模式） | `Source/Core/AppState.*` | 骨架 ValueTree 模式 × 数据中心字段 |
| FIR 卷积（声音不对、切耳机爆音） | `Source/DSP/CorrectionEngine.*` | 数据中心原逻辑，数据库注入 |
| 翻译度混合（0~200 增益/相位） | `Source/DSP/WetDryMixer.*` | 骨架槽位 × 数据中心 SIMD 混合 |
| FIR 系数表（25MB, 1603 副） | `Source/dsp/fir_data.h` | 硬链接 → 仓库A |
| 反馈上传（Supabase） | `Source/Net/FeedbackClient.*` | 数据中心原样 |
| 信号链装配、监听模式、宿主BPM | `Source/PluginProcessor.*` | 数据中心原逻辑 |
| 主界面装配/内联搜索/1h计时 | `Source/PluginEditor.*` | 数据中心 1641 行收口为 ~400 行 |
| 诚印章 | `Source/UI/SealComponent.*` | 数据中心原样 |
| 蛇形旋杆 | `Source/UI/SnakeKnob.*` | 数据中心 KnobLNF 原样 |
| 引导向导（2步：耳机→曲线） | `Source/UI/SetupCard.*` | 数据中心 + 重叠修复 |
| 过渡卡 | `Source/UI/TransitionCard.*` | 数据中心原样 |
| BPM 助手 | `Source/UI/BpmPanel.*` | 数据中心原样（拍相对换算） |
| 反馈卡 / 1小时检查 / 分享卡 | `Source/UI/{Feedback,Check,Share}Card.*` | 数据中心原样 |

## 三、信号链与状态流

```
音频线程：输入 → captureDry → FIR卷积(始终ON) → mixInPlace(0~200) → 监听模式模拟 → 输出
UI 线程 ：旋杆 → Processor.setTranslationDegree → mixer(atomic) + AppState(持久化)
持久化  ：AppState(ValueTree) ↔ get/setStateInformation（度/耳机/引导/模式/曲线/声卡）
```

## 四、剔除的死代码（数据中心确认无人调用）

- `CorrectionSwitch` 校正开关（文件1：无开关，拧到 0 即 bypass）
- `HeadphoneBrowser` 三列浏览器组件（终局用内联搜索；其静态表已迁入 HeadphoneDatabase）
- `CorrectionEngine::getProfileList()`
- 骨架 DSP：IIR CorrectionProcessor（文件1 标记"10 带参猜，已弃"）、APVTS 参数层（文件1：无 DAW 自动化）

## 五、文档 vs 实现漂移（已按"实现为准"落地，需回切只改常量）

| 项 | 文档 v0.4.0 | 实现（BETA2 采用） | 回切开关 |
|:---|:---|:---|:---|
| 画布宽 | 屏宽 23% | 屏宽 28% | `Design.h::kScreenWidthPercent` |
| 布局基准 | 600px | 420×300/645 | `Design.h::kRefWidth/Height*` |
| 向导步数 | 3 步（含声卡） | 2 步（耳机→曲线） | `SetupCard` |
| 设备选择 | 全屏三列浏览器 | 内联搜索 | 浏览器组件已剔除 |
| BPM 预延迟 | 骨架固定 16/32/64ms | 拍相对 ms/32/16/8 | 数据中心胜出，骨架 BPMCalculator 未并入 |

## 六、拼接中修复的问题

1. **SetupCard 输入框与预设行重叠渲染**（A 版现存视觉 BUG）→ 输入置顶，预设/结果互斥显示
2. **FIR 表 3 份静态拷贝**（24MB 二进制）→ 单翻译单元持有，二进制 11MB
3. **引擎默认停在 0 号耳机**（1MORE MK801）→ 编辑器构造时对齐 AppState 全名
4. **timerCallback 函数级 static**（多编辑器实例串扰）→ 成员变量 `lastDegreeShown_`
5. **listenMode 数据竞争**（UI 写/音频读裸枚举）→ `std::atomic<int>`
6. **曲线/声卡选择仅会话内** → 随 AppState 持久化

## 七、构建 / 部署命令

```powershell
$cmake = "D:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
cd C:\Users\Administrator\ZCodeProject\HonestMix
& $cmake -S . -B build -G "Visual Studio 18 2026" -A x64
& $cmake --build build --config Release --target BETA2_VST3 --parallel
rm -rf "C:\Program Files\Common Files\VST3\BETA2.vst3"
cp -r "build\BETA2_artefacts\Release\VST3\BETA2.vst3" "C:\Program Files\Common Files\VST3\"
```

## 八、两个链接（停电/换机后如何重建）

- `third_party/JUCE` → 目录联接（junction）指向仓库A的 JUCE：
  `mklink /J "...ZCodeProject\HonestMix\third_party\JUCE" "...repos\HonestMix\third_party\JUCE"`
- `Source/dsp/fir_data.h` → 硬链接指向仓库A的 25MB 数据（重新生成 FIR 后自动同步）：
  `mklink /H "...ZCodeProject\HonestMix\Source\dsp\fir_data.h" "...repos\HonestMix\Source\dsp\fir_data.h"`

## 九、已知遗留（不阻塞 BETA2）

- 仓库A `tests/UITest.cpp` 未移植（下步可挂 `juce_add_test`）
- 构建有 C4244/C4996 警告（数据中心同款，无行为影响）
- HonestMix.vst3 旧版仍在系统目录，与 BETA2.vst3 并存不冲突（UID 不同）

---

## 十、实测修复记录（DAW 实测 → 修 BUG 循环）

### 第 2 轮（6 项）
1. BPM 面板"收起"按钮不可见（alpha 0.06）→ 描边胶囊按钮
2. 三信息行点击无反应 → 根因：曲线/声卡分支从未填充结果 + content_ 吞点击；统一 `content_.addMouseListener(this,true)` 事件入口，点击即开下拉、点行即选
3. 文字看不清 → `Design.h` 令牌全局提亮（textMain 0.45→0.88 等 7 项）
4. 过渡卡狭窄 → 与向导同宽 396、按钮同几何
5. BPM/诚不对称 → 诚去方印改纯文字；同 y(244)、同尺寸(80×56)、同边距(24)
6. 检查卡偏左 → 水平居中；文案改"抱歉打扰 需要切换监听方式吗"

### 第 3 轮（3 项）
1. 耳机页出现两次 → 根因：第 8 行预设(K701)与"下一步"按钮重叠 20px 吞点击；重排布局零重叠
2. 无退回键 → 步骤 2/3 加"‹ 返回"+ 步骤指示 n/3
3. 向导改为 3 步：耳机 → 混音时长(30/60/120/180min，驱动换监听提醒，持久化 `AppState::mixHabitMinutes`) → 目标曲线

### 第 4 轮（6 项）
1. 引导时窗口拉黑长 → `fitWindow()` 按可见内容自适应高度（替代 setExpanded/collapseIfIdle 一刀切 645）
2. 卡片偏上+要毛玻璃 → 5 卡全部水平+垂直居中；`hm::blurredBackdrop()` 快照+高斯模糊铺底（Apple frosted glass）
3. 曲线注释不清 → 10px textDim → 11px textLabel
4. 过渡卡挤、说明不统一 → 手动行距 26；三条说明统一「名称 · 短描述」
5. 下拉与主界面撞车 → 打开时隐藏旋杆/数值/BPM/诚 + 面板不透明 + 底部收起提示
6. 背景纯黑 → 专业灰 (46,48,53)（"跟随宿主背景"无解：VST3 无读宿主 UI 色 API）

*第 4 轮存档 · 编译 0 错误 · 已部署 C:\Program Files\Common Files\VST3\BETA2.vst3*
