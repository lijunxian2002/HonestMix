# HonestMix BETA2 — 项目交接书

> 最后更新：2026-07-22
> 会话名：「最终完善」

---

## 一、拼接项目

**路径**：`C:\Users\Administrator\ZCodeProject\HonestMix`
**Git**：6 个 commit 存档，工作区干净

```
commit b8433f3 (HEAD -> main) 第6轮: COMPRESSOR移除
commit 3b42121                  第5轮: BPM面板重做
commit 37ad738                  BETA2 v0.4.0 初始拼接
```

**源文件**：28 个，构建 0 错误
**二进制**：11MB → 已部署 `C:\Program Files\Common Files\VST3\BETA2.vst3`

---

## 二、模块定位（修 BUG 查这里）

| 症状 | 文件 |
|:---|:---|
| 颜色/字体/布局比例 | `Source/Core/Design.h` |
| 耳机数据库/搜索/曲线表 | `Source/Core/HeadphoneDatabase.*` |
| 持久化字段 | `Source/Core/AppState.*` |
| FIR 卷积 | `Source/DSP/CorrectionEngine.*` |
| 翻译度混合 0~200 | `Source/DSP/WetDryMixer.*` |
| 反馈上传 | `Source/Net/FeedbackClient.*` |
| 信号链装配/监听模式 | `Source/PluginProcessor.*` |
| 主界面/下拉/1h计时 | `Source/PluginEditor.*` |
| 诚印章 / 蛇形旋杆 | `Source/UI/Seal.*` / `Source/UI/SnakeKnob.*` |
| 向导3步 / 过渡卡 | `Source/UI/SetupCard.*` / `Source/UI/TransitionCard.*` |
| BPM 助手 | `Source/UI/BpmPanel.*` |
| 反馈 / 检查 / 分享卡 | `Source/UI/{Feedback,Check,Share}Card.*` |

---

## 三、6 轮修复记录

### 拼接
- 骨架定结构 × 数据中心实现
- 1641 行单体 → 28 模块
- FIR 表 3 份拷贝 → 1 份（24MB→11MB）
- 剔除 4 处死代码（CorrectionSwitch、Browser、getProfileList、IIR）

### 第 2 轮
1. BPM 关闭按钮不可见 → 描边胶囊
2. 三行点击无反应 → 事件统一 + 曲线/声卡分支填充
3. 文字看不清 → Design.h 7 项提亮
4. 过渡卡狭窄 → 396×400
5. BPM/诚不对称 → 纯文字印章 + 同尺寸
6. 检查卡偏左 → 居中 + 文案改

### 第 3 轮
1. K701 吞按钮 → 布局零重叠
2. 退回键 → "‹ 返回"
3. 向导 3 步（耳机→混音时长→曲线）

### 第 4 轮
1. 窗口拉黑长 → fitWindow 自适应
2. 卡片偏上 → 垂直居中 + 毛玻璃（高斯模糊底）
3. 曲线注释 → 11px textLabel
4. 过渡卡挤 → 行距 26 + 统一格式
5. 下拉撞车 → 隐藏元素 + 不透明 + 收起提示
6. 纯黑底 → 专业灰 (43,45,50)

### 第 5 轮
1. BPM 过大 → 固定 380×522 + 字体加大
2. 毛玻璃底 → 面板模糊
3. 收起钉右上
4. 均衡布局 → 比例列
5. 曲线加特点说明
6. 灰金配色

### 第 6 轮
1. 压缩释放移除 → 面板 460
2. 数值右对齐
3. BEAT → 每拍
4. 改用 Unicode 中文

---

## 四、UI 终稿（2026-07-22 定案）

### 布局：600px 宽 · 左仪表 右推子

| 区域 | 内容 |
|:--|:--|
| **左栏** | Frequalizer 风格频谱面板：dB/Hz 网格 + RAW(红)/已校正(绿)/TGT(灰) 三曲线，支持点击/悬停 Hz+dB tooltip |
| **右栏** | 垂直推子：上 π 下 0，36×200px 金属轨道，地球仪=推子帽（NASA 纹理，46px，中国居中） |
| **底部** | 信息芯片 (HP/曲线/IF) + BPM / MONITOR / 诚·反馈 |

### 核心交互

| 交互 | 效果 |
|:--|:--|
| 推子上下拖动 | 地球旋转（底=背面0，顶=中国π）+ 曲线红→绿实时交叉淡化 + LED 呼吸 |
| 点击曲线/图例 | 选中曲线，悬停显示 Hz+dB 实时数据 |
| 值 0→π | 面板 LED：红(RAW)→黄绿(过渡)→亮绿(MAX)，三段呼吸动画 |

### 视觉系统

| 要素 | 值 |
|:--|:--|
| 底色 | 深空黑 `#0f1016` |
| 面板 | 6 颗 LED 指示灯，3 种节律呼吸 |
| 曲线 | SVG feGaussianBlur 发光 + opacity 交叉淡化 |
| 灯光 | LED 点阵风格，非软光晕 |
| 地球 | NASA 真实纹理，`earth_texture.jpg` 1.4MB，桌面同级 |

### 桌面参考文件

| 文件 | 用途 |
|:--|:--|
| `HonestMix_Frequalizer方向_终版.html` | **★ 终版预览** 完整交互原型 |
| `earth_texture.jpg` | NASA 地球纹理（HTML 依赖） |
| `插件UI参考_三款开源.html` | 早期参考（存档） |
| `精密仪器风格_UI方向.html` | 中期方向（存档） |
| `开源插件UI验证_精密仪器方向.html` | GitHub 验证报告（存档） |

---

## 五、待完成

- [ ] **Design.h 落地**：深空黑底 + LED 辉光令牌 + 曲线面板配色
- [ ] **PluginEditor 布局重构**：左仪表右推子 600px 宽，GridLayout 双列
- [ ] **SnakeKnob → 垂直推子控件**：地球仪推子帽，拖拽旋转地球
- [ ] **校正曲线图**：Frequalizer 风格实时频响显示，dB 标尺 + Hz 对数轴
- [ ] **曲线交叉淡化**：推子联动 RAW→CORR opacity 过渡
- [ ] **LED 呼吸系统**：6 颗指示灯，3 段状态，3 种节律
- [ ] **DAW 实测**：编译部署，验证交互

---

## 六、桌面文件

| 文件 | 用途 |
|:---|:---|
| `ChatGPT自用.bat` | 一键连 API，设环境变量，开聊天页 |
| `插件UI参考_三款开源.html` | 三款开源项目 UI 风格预览（CSS 模拟） |
| `复古暖色硬件质感_UI参考.html` | 复古暖色三流派对比（已否决方向，存档） |
| `硬件质感实现方案_参考.html` | 纹理+阴影+噪点的硬件化方法（参考） |
| `精密仪器风格_UI方向.html` | **★ 当前方向** 精密仪器设计预览 + Design.h 代码 |
| `UAD级重设计_HonestMix.html` | UAD Plate 风格对比分析（已否决方向，存档） |
| `开源插件UI验证_精密仪器方向.html` | **★ 验证报告** 8 个 GitHub 项目对照分析 |
| `HonestMix_风格预览对比.html` | **★ 预览** HonestMix × 各参考风格的模拟效果 |

API 配置在 bat 文件顶部：
- API_BASE = `https://chaofanshou.com/v1`
- API_KEY = `sk-30a12d1ae6ba130031841f62ff4a51121b38270ec71f5e9f5eb5008a92cb4556`
- MODEL = `gpt-5.5`

---

## 七、数据中心

**路径**：`C:\Users\Administrator\source\repos\HonestMix`
- 原始仓库（git），未动一行
- 含完整 git 历史、third_party/JUCE（供应商）
- 本通过 `third_party/JUCE` 目录联接到拼接项目

---

*下次回来提"最终完善"即可继续。*
