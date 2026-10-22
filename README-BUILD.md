历史0.6.6 补充：源码Verify-Local还会运行 tools/profile-generation/verify.mjs，逐档校验1573个原始CSV/目标SHA并生成新临时header，要求与编译header逐字节相同。FidelityCheck为原6档加K712 Dekoni×4rate共28行；ResponseContract同样增加该档案9个0–200点及真实正弦/曲线，不放宽原阈值。独立NumPy检查器可使用 python tools/profile-generation/independent-oracle.py NEW_REPORT.json 运行，需NumPy；其实际结果随源码保存。

# HONESTMIX Headphone 0.6.10 候选构建与交付

版本唯一来源为 CMakeLists.txt 的 project VERSION。开发候选在 C 盘；JUCE 可用开发机 D 盘 checkout，最终对应源码 ZIP 必须自带 deps/juce。不复用其他 checkout 的 build 目录。所需环境：Windows x64、CMake≥3.22、Ninja、MSVC C++、Windows SDK、Node.js；打包另需 7-Zip（HM_7Z 可指定 exe）。不联网下载依赖。

以下为0.6.10候选的复现要求。本版调整主界面入口、推子说明和悬停/星空动态；音频DSP、测量数据与参数语义保持原样。通过状态须核对本版实际报告，不由本文预先授予。交付须重建、完成四套原生QA、动态界面截图审查、输出峰值专项、外部VST3验证、冻结、打包、实际源码提取重编与私有整包安装/备份/回滚。旧版报告不能改名充当本版通过证据。

## 构建

在配置好 MSVC 的终端、工程根目录执行：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DHM_JUCE="D:/CodexWork/sonobus-cn-20261015/src/deps/juce"
cmake --build build --target HonestMixHeadphone_Standalone HonestMixHeadphone_VST3 HeadphoneTests FidelityCheck ResponseContractTests TempoRecallTests
```

提取出的源码改用 -DHM_JUCE="<提取源码目录>/deps/juce"，不使用开发机绝对路径。Source/、tests/ 整树收集，包含 CorrectionResponse、IRConversion、TempoModel、ResponsePlotModel、MeasuredHeadphone、HeadphonePrediction 与四套原生QA，不能挑旧文件覆盖。

## 当前证据、冻结与打包

以下 PowerShell 命令以工程根为当前位置。每个程序结束都检查退出码；失败时保留报告并停止，不生成通过证据。首次创建 evidence/final，使用新目录复验可保留旧结果。

```powershell
New-Item -ItemType Directory -Force evidence/final | Out-Null
& "build/HeadphoneTests_artefacts/Release/HeadphoneTests.exe" "$PWD/evidence/final" "$PWD/build/HonestMixHeadphone_artefacts/Release/VST3/HONESTMIX Headphone.vst3"
if ($LASTEXITCODE -ne 0) { throw "HeadphoneTests failed" }
& "build/FidelityCheck_artefacts/Release/FidelityCheck.exe" "$PWD/evidence/final/fidelity-check.json"
if ($LASTEXITCODE -ne 0) { throw "FidelityCheck failed" }
& "build/ResponseContractTests_artefacts/Release/ResponseContractTests.exe" "$PWD/evidence/final/response-contract.json"
if ($LASTEXITCODE -ne 0) { throw "ResponseContractTests failed" }
& "build/TempoRecallTests_artefacts/Release/TempoRecallTests.exe" "$PWD/evidence/final/tempo-recall"
if ($LASTEXITCODE -ne 0) { throw "TempoRecallTests failed" }
node tools/freeze-delivery.mjs
if ($LASTEXITCODE -ne 0) { throw "Freeze failed" }
node package.mjs --preflight
if ($LASTEXITCODE -ne 0) { throw "Preflight failed" }
node package.mjs
```

四份报告和截图默认取 evidence/final。上面的freeze/package步骤还以前置的独立pluginval验收为条件：对最终完整VST3目录执行等级5，保留实际工具身份、命令、退出码、当前VST3 SHA及全部日志到evidence/final/external-vst3，validation.json绑定这些实测结果。仅编译/内部宿主测试不足以产生该回执；UI-only预览也不能用于打包。发行协调器负责生成并审核这份外部宿主记录，未完成时preflight拒绝交付。原生测试必须同时给证据目录和 VST3 路径，否则缺少完整 UI/VST3 记录。FidelityCheck 没有独立 version 字段，依靠新鲜度、当前测试 exe、源码和最终冻结 SHA 绑定；不接受上午的 fidelity-fix 补丁合入报告代替当前实测。

自定义路径时，freeze 与 package 使用同一组选项：--report <headphone-tests.json>、--evidence-dir <截图目录>、--fidelity-report <fidelity-check.json>、--response-report <response-contract.json>、--tempo-report <tempo-recall/result.json>、--freeze-manifest <新冻结清单>、--juce <依赖目录>。显式 JUCE 必须与当前 CMakeCache 的 HM_JUCE 一致。

冻结工具不运行测试；它记录全部源码、交付脚本/说明/许可、实际 JUCE、六个二进制（含 VST3）及资源、四报告、UI 截图的 SHA。冻结清单不覆盖，输入变化后应重跑受影响验证并使用新清单；不能仅改清单消除失败。打包再次逐项比对，禁止使用 --skip-optional-qa 绕过 FidelityCheck/ResponseContractTests/TempoRecallTests。EXE、VST3、四 QA 的 PE 版本及 VST3 元数据必须等于 CMake 版本，二进制与报告必须晚于其真实相关 Source 和本目标测试源。纯 CMake 测试目标/脚本调整以冻结配置哈希、版本及真实 configure/build 记录绑定，不要求无关产品虚假重链。必要时由构建负责人修复资源版本依赖后重编，不篡改 PE 版本。

打包要求HeadphoneTests全部原生子段、多尺寸截图、内嵌TEMPO操作与真实VST3扫描实例化通过；新增presentationMotion子段须通过，且away/hover/left/faded四状态的100%/200%共8张实际原生截图齐全；主界面正好保留两个双语动作，原独立离线指南纯导出回归保留；FidelityCheck的7档案×4采样率共28组均须满足绝对幅度0.01dB/相位0.001rad门槛；ResponseContractTests须覆盖响应、档案切换、失败重试、监听、旁路、参数/宿主速度及预计耳机频响；TempoRecallTests的14项页面记忆、旧工程、非法值和HOST生命周期检查须通过。各测试实际覆盖、阈值与数值以当前源码和报告为准；本段是交付要求，不是0.6.10通过声明。C++ new计数不是所有分配入口的总量。

## 包结构与提取验证

release/HONESTMIX-Headphone-<版本>-Windows-x64.zip 包含 client、完整 plugin VST3、qa 下四个测试 exe、licenses、当前 evidence、使用/构建/远程联调三份README、对应源码 ZIP 与 SHA256.json。源码 ZIP 含完整 Source/tests/tools/licenses、CMakeLists/build.cmd/package、三份README、完整 deps/juce 与 SOURCE-SHA256.json。开发机绝对依赖路径仅作来源记录，提取配置必须显式使用包内 JUCE。

先用 7-Zip 将外层 ZIP 解到一个新目录，再执行：

```powershell
node "<解出的包目录>/tools/verify-delivery.mjs" "<解出的包目录>" --extract-to "<全新的源码验证目录>" --configure
```

该工具逐文件核对发行包清单、源码 ZIP hash、源码内部清单和最终冻结源码/JUCE hash，再用包内 JUCE 做 configure 并核查 CMakeCache。配置需要上述本机构建环境；验证工具不安装环境、不清理目录、不自动重编音频。提取目录必须不存在。交付负责人随后必须在提取目录用包内JUCE全新构建EXE、完整VST3与四个QA程序，并运行Verify-Local.ps1和实际输出峰值专项；核对六个PE版本、完整原生报告、峰值构建/运行身份及外层ZIP前后SHA。还须在独立私有扫描目录对完整VST3做安装、旧版备份和精确回滚；真实宿主/全局插件目录不得作为该本地验证的目标。configure成功不能当成全套运行通过。

ZIP 继续使用 7-Zip -mcu=on，并核查所有中文条目的中央目录 UTF-8 位。每次打包使用全新 owner-token stage；release 同名产物拒绝覆盖。默认 stage 为工程 tmp，--stage-root/HM_STAGE_ROOT 可指定其他盘；空间不足拒绝继续。脚本不删除 build/Source 或清理磁盘，stage 仅在显式 --discard-stage <目录> --token <token> 且边界/无链接/owner 全部匹配时清理。失败 stage 保留。

## 验收边界

离线音频、离屏 UI、内部 VST3 扫描的证据与真实声卡、第三方 DAW、真实系统 DPI 分开。源码及 JUCE 的 GPL/商业许可见 licenses；测量数据、目标曲线的商业授权尚未核验。旧版数值与报告保留历史含义，不复写成当前候选最终结果。


## 历史0.6.4 配色交付证据范围

0.6.4当时改变经审查的颜色字面量和项目版本；HEADPHONE 另含独立审定的临时不可用状态文案修复，曲线继续隐藏不可信数据，处理器和DSP不变。新二进制已完成随包列明的原生UI/交互/最小运行及独立VST3验证。旧版 0.6.3 的完整DSP/传输验收及真实 90 分钟独立长会话结果按未变源码SHA继承；HEADPHONE为原90分钟，CONNECT为预声明修正QA的30分钟墙钟（含控制和重连），旧R2和诊断失败原样保留。旧报告保持原版本；新EXE本身没有运行上述长会话。源码包自带依赖并实际提取配置；未宣称本轮新提取源码已再次全量编译。真实声卡、第三方DAW和公网TLS/客户设备仍未在本轮验收。

## 交付必需的输出峰值专项

当前源码包含 tests/PeakTelemetryProbe.cpp，CMake的HM_BUILD_PEAK_PROBE可启用同名目标。正式打包还要求当前SharedCode的真实编译/链接绑定、探针运行报告、100%/200%全部26张界面图和逐文件验证清单，放入evidence/final/peak-telemetry并通过tools/peak-telemetry-contract.mjs。发行协调器可使用包内evidence/peak-telemetry/build-probe.py按同一构建环境生成独立探针，再以tools/stage-peak-telemetry.mjs收集实际结果；须按脚本实际参数执行并检查退出码。该专项只调用离线合成音频及界面渲染；失败目录不得覆盖，四套常规QA仍必须完整通过。新提取源码也须重新构建并运行对应专项，不复用开发checkout的结果。
