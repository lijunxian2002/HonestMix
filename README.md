# HonestMix 诚听

> 代码 GPLv3 开源 · "HonestMix / 诚听"名称为商标（申请中），衍生产品请勿使用本名称 → [TRADEMARK.md](TRADEMARK.md)

**A free headphone-correction and monitor-translation plugin (VST3/AU), built in Chinese, for the hundreds of thousands of home-studio producers who mix on headphones alone.**

> Status: **working code, pre-release. Version 0.6.12** builds on Windows x64
> (VST3, JUCE). The correction engine, headphone database, VST3 state restore,
> BPM recall and a 13-file native test suite are in `Source/` and `tests/`.
> No signed installers, no independent security audit, no public release yet.
> The roadmap below marks what actually builds today, not what is planned.

---

## Why this exists

Most home-studio producers in China mix entirely on headphones. Speakers and
treated rooms are simply beyond their budgets. Headphone mixing suffers from
three structural distortions:

1. **Frequency response** - every headphone colors the sound (solvable with measurement + filtering)
2. **Missing crosstalk** - speakers reach both ears, headphones do not (addressed with HRTF filtering)
3. **Missing room** - the brain uses early reflections to judge depth and reverb (approximated with BRIR convolution)

The result: mixes that do not translate, and long sessions at ever-rising
listening levels. We want to fix what software can honestly fix - no more,
no less.

## What we are building

- VST3/AU plugin on the master bus, near-zero learning curve
- Headphone correction profiles (domestic Chinese models included and prioritized)
- HRTF crossfeed + measured-room simulation
- Translation checks (phone speaker / car / small room)
- SPL calibration and gentle listening-exposure awareness as first-class features
- Chinese UI, Chinese tutorials, 人民币 pricing if ever commercial

## Ethics and data (read this first)

- This repository ships **517 headphone profiles, all measured by oratory1990**,
  included with the measurement author's permission for free non-commercial
  redistribution. Nothing else — full inventory and removals in [DATA-RIGHTS.md](DATA-RIGHTS.md).
- An earlier build carried 1,573 profiles from eleven sources. The 1,056 we had
  no redistribution permission for — including all 17 Rtings curves, a source
  that forbids this use — were **removed from the repository**, not left in the
  data table behind a disclaimer.
- We use headphone measurement data **only with the explicit permission of its authors**,
  and we credit them in the plugin and documentation from day one.
- A public download URL is not a licence, and traceable provenance is not permission.
- The project is **completely free and non-commercial during its public testing phase**.
  If it ever becomes commercial, we will return to every data author first and
  accept whatever licensing terms they consider fair.
- No medical claims. This tool cannot prevent ear disease; it can only help
  people listen at saner levels.
- Headphone model names remain trademarks of their owners.

## We will prove it - or publish that it failed

We pre-commit to a blind evaluation protocol:

- 20-30 experienced home-studio producers each mix the same song twice,
  with and without the plugin, order randomized.
- Both versions are judged blind on five playback systems
  (studio monitors / car / phone / bluetooth speaker / another headphone).
- **Win rate >= 60%**: the tool works. **50-60%**: iterate. **< 50%**: we stop
  and publish the data anyway, so the community learns from it either way.

## Roadmap - actual state

| Phase | Content | State |
|-------|---------|-------|
| M0 | Trademark and licensing decisions | done (GPL-3.0, `TRADEMARK.md`) |
| M1 | Correction engine, headphone database, bypass A/B | **done in 0.6.12** |
| M1b | HRTF crossfeed | not started |
| M2 | Measured-room simulation, SPL calibration, UI freeze | UI built; room and SPL not started |
| M3-4 | Blind internal test (20-30 producers) | not started |
| M5-7 | Free public beta, monthly iteration | not started |
| M8 | Data-driven decision: monetize / stay free / keep open | not started |

## Build

Requirements:

- Windows x64
- CMake 3.22 or newer
- Visual Studio 2022 with the C++ desktop workload
- JUCE (GPL branch) sources

Dependencies are **not** vendored in this repository. `CMakeLists.txt` expects
them under `deps/`, or pass `-DHM_DEPS=<path>`:

```
deps/
  juce/       JUCE source tree with CMakeLists.txt
  windows/    Windows SDK shims used by the include path
```

```bat
build.cmd
```

That produces the VST3 plugin and the console test binary. `README-BUILD.md` has
the detailed toolchain notes; `Verify-Local.ps1` runs the native test suite and
writes its results to a fresh `verification-*` directory.

## Repository layout

```
Source/              plugin processor, editor, correction engine
Source/dsp/          FIR coefficient data and correction response
Source/UI/           skins, star field, components, measured response data
tests/               13 native test files (response contracts, state restore,
                     tempo recall, fidelity, update link, UI probes)
tools/               headphone-profile generation and delivery verification
docs/                project documentation
licenses/            third-party license texts
CMakeLists.txt       JUCE build definition
```

## Who

**JunXian Li** - mixing engineer, SOLAR&TERRA DESIGN STUDIO, China.
Issues and discussions are open. 中文也完全没问题。

---

# 中文版

**诚听**是一款免费的中文耳机监听矫正插件（VST3/AU），为只能用耳机混音的
home studio 制作人而生。

- **三层失真全部覆盖**：频响矫正、HRTF 串扰、实测房间模拟
- **翻译检查**：一键切换手机外放 / 车载 / 小房间
- **听力健康**：SPL 校准与聆听暴露量提醒，做成功能而非口号
- **伦理先行**：本仓库只包含 oratory1990 授权可再分发的 **517 副**数据，明细见
  [DATA-RIGHTS.md](DATA-RIGHTS.md)；早期版本另有 1,056 副未获授权（含 17 副明确
  禁止此类使用的 Rtings 曲线），已全部从仓库移除，而不是留在数据表里加一句免责；
  公测期完全免费非商用；若将来商用，先回到每位数据作者面前接受其公平的授权条款
- **用盲测说话**：20-30 位制作人双盲混音对比，五种回放系统盲评，
  事先承诺判定标准——胜率不到 50% 就停止并公开全部数据

**当前阶段：0.6.12 可编译的预发布版。** 频响矫正引擎、耳机数据库、VST3
状态恢复、BPM 记忆和 13 个原生测试文件都已在本仓库。HRTF 串扰、实测房间
模拟和 SPL 校准尚未开始；没有签名安装包，没有独立安全审计，尚未公开发布。

发起人：李俊贤，混音师，太阳与大地设计工作室（SOLAR&TERRA DESIGN STUDIO）。
