# Third-Party Notices — HonestMix

本文件记录 HonestMix 使用的第三方代码与数据来源及其许可条款。
**HonestMix 免费开源（GPLv3），严格非商用。**

---

## 1. JUCE Framework

- **作者**: Raw Material Software / JUCE
- **许可**: GPLv3（本项目按 GPLv3 开源使用）
- **用途**: 音频插件框架（VST3/Standalone）

## 2. AutoEq（FIR 生成管线）

- **作者**: Jaakko Pasanen
- **许可**: MIT License
  ```
  MIT License
  Copyright (c) 2018-2022 Jaakko Pasanen
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software...
  ```
- **用途**: 耳机测量数据 → FIR 矫正滤波器（`Source/dsp/fir_data.h` 由 AutoEq 管线生成）

## 3. Headphone Measurements: oratory1990（Konstantin Davy）

- **作者**: Konstantin Davy（oratory1990）
- **许可**: 免费非商用使用（作者授权，2026-08-22 邮件确认）
- **商用**: 需单独签订授权协议（作者标准条款）
- **署名**: 每副耳机数据源已标注 `source: oratory1990`
- **支持**: https://buymeacoffee.com/oratory1990（捐赠支持其测量设备维护）
- **用途**: 耳机频率响应测量数据（约 1570+ 型号）

## 4. Harman Target Curves

- **来源**: Harman International 参考目标曲线（oratory1990 / AutoEq 发布的数值数据）
- **用途**: 矫正目标曲线参考

---

## 合规说明

1. HonestMix 公测期完全免费、开源、非商用——符合 oratory1990 数据免费条款
2. 数据来源逐条署名（`Source/dsp/fir_data.h` 中 `source:` 标注）
3. 禁用数据源（如 RTINGS 等禁止商用来源）不包含在本项目
4. 若 HonestMix 将来转为商用，将先与 oratory1990 签订正式授权协议

*更新：2026-08-22（oratory1990 授权确认）*
