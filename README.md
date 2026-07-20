# HonestMix 诚听

> 代码 GPLv3 开源 · “HonestMix / 诚听”名称为商标（申请中），衍生产品请勿使用本名称 → [TRADEMARK.md](TRADEMARK.md)

**A free headphone-correction and monitor-translation plugin (VST3/AU), built in Chinese, for the hundreds of thousands of home-studio producers who mix on headphones alone.**

> Status: early design stage (pre-alpha). This repository currently hosts the project
> book and research plan. Code will follow.

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
- Chinese UI, Chinese tutorials,人民币 pricing if ever commercial

## Ethics and data (read this first)

- We use headphone measurement data **only with the explicit permission of its authors**,
  and we credit them in the plugin and documentation from day one.
- Sources known to forbid commercial use (e.g., RTINGS) will not be used at all.
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

## Roadmap (rough)

| Phase | Content |
|-------|---------|
| M0 | Trademark & licensing decisions, DSP latency spike |
| M1 | MVP: correction + crossfeed + bypass A/B |
| M2 | + first measured room, SPL calibration, UI freeze |
| M3-4 | Blind internal test (20-30 producers) |
| M5-7 | Free public beta, monthly iteration |
| M8 | Data-driven decision: monetize / stay free / open-source |

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
- **伦理先行**：测量数据只在获得作者明确授权后使用并署名；禁商用来源
  （如 RTINGS）一概不碰；公测期完全免费非商用；若将来商用，先回到每位
  数据作者面前接受其公平的授权条款
- **用盲测说话**：20-30 位制作人双盲混音对比，五种回放系统盲评，
  事先承诺判定标准——胜率不到 50% 就停止并公开全部数据

当前阶段：设计期（pre-alpha），本仓库先存放项目书与研究计划，代码随后。

发起人：李俊贤，混音师，太阳与大地设计工作室（SOLAR&TERRA DESIGN STUDIO）。
