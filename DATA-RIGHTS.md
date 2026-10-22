# Measurement data rights

## What this repository contains

**517 headphone correction profiles, all measured by oratory1990**, included
with the measurement author's permission for free, non-commercial
redistribution.

Nothing else. The count is the same in all three places:

- `tools/profile-generation/inputs/` — 517 CSV files
- `Source/dsp/fir_data.h` — 517 FIR coefficient sets, `NUM_PROFILES = 517`
- `Source/UI/measured_data.h` — the same 517 magnitude curves, `profiles=517`

## What was removed, and why

An earlier working build carried 1,573 profiles from eleven sources. Only one
source had cleared redistribution. The other 1,056 profiles are **not** in this
repository:

| Source | Profiles | Status |
|---|---|---|
| Innerfidelity | 531 | no redistribution permission obtained — removed |
| **oratory1990** | **517** | **permission granted — included** |
| Kuulokenurkka | 190 | no redistribution permission obtained — removed |
| Filk | 92 | no redistribution permission obtained — removed |
| Headphone.com Legacy | 86 | no redistribution permission obtained — removed |
| Super Review | 70 | no redistribution permission obtained — removed |
| kr0mka | 31 | no redistribution permission obtained — removed |
| **Rtings** | **17** | **explicitly forbids this use — removed** |
| Auriculares Argentina | 17 | no redistribution permission obtained — removed |
| Regan Cipher | 14 | no redistribution permission obtained — removed |
| HypetheSonics | 8 | no redistribution permission obtained — removed |

A public download URL is not a license. Traceable file provenance is not
permission either. We would rather ship 517 profiles we can defend than 1,573
we cannot.

The Rtings rows deserve a specific note: this README previously said sources
that forbid this use would not be touched, while the data table still carried
17 Rtings curves. That was a contradiction, not an oversight of wording. The
curves are gone.

## Rules for anyone adding data

1. Get written permission from the measurement author **before** adding a profile.
2. Record it in `tools/profile-generation/provenance.json` together with the
   exact input file SHA-256.
3. Never add a source whose terms forbid redistribution, however easy the file
   is to download.
4. If a source's terms change, remove the data in the next release and say so
   in the changelog.

## What this does not claim

- Not an independent measurement of any physical unit.
- Not a calibrated reference. The plugin corrects; it does not certify.
- No medical claim. Nothing here prevents hearing damage.

## 中文摘要

本仓库只包含 **oratory1990 测得的 517 副耳机数据**，经其授权用于免费非商用分发。
早期版本曾包含 11 个来源共 1,573 副，其中仅 oratory1990 一家取得再分发授权，
其余 1,056 副（含 17 副明确禁止此类使用的 Rtings 曲线）已全部从仓库中移除。
公开可下载不等于授权，可追溯的来源也不等于许可。
