# HonestMix 诚听

一款开源（GPL-3.0）、免费的中文耳机频响校正 VST3 插件。

让卧室制作人用普通耳机听到接近标准监听环境的声音。

---

## 构建说明

### 环境
- Visual Studio 2026 Community（"使用 C++ 的桌面开发"工作负载）
- CMake 4.3+
- JUCE 8（`master` 分支，自动克隆到 `third_party/JUCE`）

### 编译

```powershell
# 1. 克隆项目
git clone https://github.com/lijunxian2002/HonestMix.git --recurse-submodules
cd HonestMix

# 2. 下载 JUCE
mkdir third_party
cd third_party
git clone https://github.com/juce-framework/JUCE.git --depth 1 -b master
cd ..

# 3. CMake 配置
cmake -B build -G "Visual Studio 18 2026"

# 4. 编译
cmake --build build --config Release

# 5. 输出
# build/HonestMix_artefacts/Release/VST3/HonestMix.vst3/
```

---

## 技术栈

- **框架**：JUCE 8（CMake 构建）
- **语言**：C++17
- **插件格式**：VST3（Windows 优先）
- **编译器**：MSVC 19.51（Visual Studio 2026）

---

## 当前进度

| 里程碑 | 状态 |
|--------|------|
| M0「有声骨架」| ✅ 编译通过，可在 DAW 加载 |
| M1「校正引擎」| ⬜ 开发中 |
| M2「空间体感」| ⬜ 待开始 |
| M3「公测版」  | ⬜ 待开始 |

---

## 许可

GPL-3.0

## 联系

开发者：李俊贤 · SOLAR&TERRA DESIGN STUDIO
