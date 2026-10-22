#pragma once
#include <juce_core/juce_core.h>

// Embedded documentation is available without a website login or network access.
namespace hm { namespace support {
inline juce::String guideEscape(juce::String text)
{
    return text.replace("&", "&amp;").replace("<", "&lt;")
               .replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&#39;");
}

inline juce::String offlineGuideHtml(const juce::String& product, const juce::String& version)
{
    const bool headphone = product.equalsIgnoreCase("Headphone");
    const auto title = juce::String("HONESTMIX ") + (headphone ? "Headphone" : "Connect");
    const auto body = juce::String::fromUTF8(headphone ? R"GUIDE(
<p class="intro">选耳机，调校正，听差异。</p>
<section><h2>开始使用</h2><ol><li>将完整 VST3 文件夹放到宿主的 VST3 目录并重新扫描；或运行完整便携包中的 EXE。</li><li>选择与实际耳机完全对应的型号。将插件放在监听链路末端，避免将监听校正重复写入交付音频。</li><li>从 <b>100</b> 开始，降低监听音量，再比较原声与校正。没有测量数据支持的型号，不应使用近似型号冒充精确校正。</li></ol></section>
<section><h2>三个按键</h2><p><b>TEMPO / 节拍</b>：时间换算。<b>COMPARE / 对比</b>：在 0 与保存的校正量间切换。<b>GUIDE / 指南</b>：本页。</p></section>
<details><summary>0–200、声音与图表</summary><p>0 为不加校正；100 为完整校正；100 以上为过校正。步进为 0.5，拖动有阻尼，按住 Shift 可精细调整。</p><p>实际混合使用 w = 数值 / 100，H = (1 − w) × 延迟对齐的原声 + w × 校正滤波器。这是复数频响的混合，并非将 dB 曲线直接倍乘，也不代表感知响度加倍。过校正可能加重峰谷，建议降低音量。</p><p>主图显示耳机的预计频响：0%是档案原始测量，100%是该测量叠加实际完整校正后的结果，100–200%同样由真实滤波器复数混合计算。目标以灰色虚线表示，不强制拉平，也不人为添加起伏。原始测量固定以1kHz作相对电平参考，各档校正不另行归一化；悬停显示同一模型的精确相对dB。超过显示刻度会提示，不会篡改数据；测量范围外不外推。此图基于档案和实际滤波器的模型估算，不是对你耳边声压的实测；佩戴及个体差异会影响结果。旧版参考滤镜启用时隐藏主图，旁路显示原始测量。</p><p><b>OUT PK</b> 提示实际输出超过0 dBFS的采样峰值，界面保留1.5秒以免漏掉短促超峰。它不是耳机相对频响、真峰值或声压读数，也不会自动限幅或补偿增益。</p><p>COMPARE 切换校正量，不是宿主硬旁通。参数变化经过平滑处理，瞬时声音以实际应用状态为准。</p></details>
<details><summary>HOST / MANUAL 与时间换算</summary><p>HOST 跟随宿主提供的有效 BPM；HOST 数据缺失或过期时不显示可信换算，需要手动切换 MANUAL。初次没有有效宿主时从 MANUAL 开始。手动数值、节拍类型与来源选择会随工程保存，关闭再打开界面也会保留。以界面显示的来源为准，不把手动数值当作宿主测量结果。</p><p>四分音符 q = 60000 / BPM 毫秒；1/n 音符为 4q/n。附点 × 1.5，三连音 × 2/3。显示值会适当舍入，计算保留输入精度。</p><p>Pre-delay：q/32、q/16、q/8。Reverb：Room q/2–q，Plate 2q–4q，Hall 4q–8q。Release：Fast q/16–q/8，Medium q/4–q/2，Slow q–2q。这些是节拍参考值，不是房间测量或混响算法。</p></details>
<details><summary>旧工程与高级监听模式</summary><p>默认使用正常立体声。右键高级菜单保留旧工程兼容模式：单声道、400 Hz 高通、100 Hz / +6 dB 低架。它们是简化的监听滤波器，不是实测手机、汽车或音箱模拟。</p><p>图表显示校正阶段，位于这些旧监听处理之前；启用旧模式时请留意界面标记。需要纯校正监听时，选择 Normal Stereo。</p></details>
)GUIDE" : R"GUIDE(
<p class="intro">连接音频，先确认每一段链路。</p>
<section><h2>开始使用</h2><ol><li>运行完整便携包中的 EXE，或安装完整 VST3 文件夹并让宿主重新扫描。</li><li>发起方在连接设置中填写可信 HTTPS 节点。创建连接后，将完整连接码私下交给协作者；加入方粘贴，沿用码内节点。</li><li>选择正确的输入与输出。宿主中将插件放入实际有音频的轨道，并确认通道及监听路由。先小音量测试，避免将接收音频反馈回发送端。</li></ol><p class="state">公网原生音频链路仍待验收；节点健康响应不代表双向音频已经连通。</p></section>
<details><summary>连接失败时按阶段检查</summary><ol><li><b>HTTPS</b>：公网发起和加入都需要 HTTPS；同电脑回环测试可用 http://127.0.0.1:端口 或 localhost。确认域名、证书、系统时间及节点地址。自检保留失败 API 阶段、即时错误码与已收到的 HTTP 状态，不代表能确定逐层根因。</li><li><b>会话</b>：确认双方连接码、节点与会话状态。网络请求失败不应显示成已连接。</li><li><b>UDP</b>：节点需要开放实际配置的 UDP 端口；常用部署为 28125。安全组、服务监听及本地防火墙需要分别确认。</li><li><b>音频</b>：检查电平、声道、宿主轨道、音频设备、缓冲及路由。状态灯或网络 RTT 不能代替真实声音测试。</li></ol><p>随包工具 tools/Test-Node.ps1 使用 -Endpoint 指定节点，可用 -JsonOut 保存报告。-TimeoutMs 默认 5000，范围 250–15000 毫秒。仅对你管理的节点运行；结果必须结合实际客户端连接及双向音频判断。-NoProxy 只影响本次 HTTP 会话，不改变系统 TUN 或路由，不能保证查明 DNS 根因。不要忽略证书验证来制造通过结果。</p></details>
<details><summary>本地节点与公网部署</summary><p>随包提供节点源码与部署说明。本地测试节点只验证该环境；公网需要可访问地址、可信 TLS、正确的 TCP / UDP 配置及部署地区要求的接入条件。</p><p>域名实名、备案与网络路径是不同环节，完成其中一项不能保证其他环节通过。公共服务应在原生客户端双向音频验证后再提供给客户。</p></details>
<details><summary>延迟、声卡与协作范围</summary><p>网络 RTT、编解码缓冲、音频设备缓冲与音频往返时间不同。音频延迟需在真实两端、目标声卡与实际宿主中测量，没有统一的全国 10 ms 保证。</p><p>目前的连接目标是音频协作，并不等于多人同步编辑同一 DAW 工程、云端工程存储或远程控制电脑。</p></details>
)GUIDE");
    return juce::String::fromUTF8(R"GUIDE(<!doctype html><html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'unsafe-inline'"><title>)GUIDE")
        + guideEscape(title) + juce::String::fromUTF8(R"GUIDE( — Guide</title><style>:root{color-scheme:dark}*{box-sizing:border-box}body{margin:0;background:#12171c;color:#d9e0e6;font:16px/1.85 system-ui,'Segoe UI','Microsoft YaHei',sans-serif}main{max-width:820px;margin:0 auto;padding:64px 28px}header{border-bottom:1px solid #39414a;padding-bottom:24px;margin-bottom:32px}small{color:#9aa9b5;font-size:12px;letter-spacing:.1em}h1{font-weight:500;letter-spacing:-.03em;font-size:36px;line-height:1.2}h2{font-size:18px;font-weight:500;margin:0 0 12px}.intro{color:#a7b9c6;font-size:20px}section{margin:32px 0}p,ol{margin:12px 0}li{padding-left:4px;margin:9px 0}b{color:#f1f6fa;font-weight:600}.state{border-left:2px solid #d3aa77;padding:4px 16px;color:#d3c2ac}details{border-top:1px solid #303b44;padding:18px 0}summary{cursor:pointer;color:#eff4f7;outline-offset:5px}details p,details ol{color:#aab9c4}footer{font-size:13px;color:#91a0ac;border-top:1px solid #303b44;margin-top:32px;padding-top:22px}@media(max-width:480px){main{padding:32px 20px}h1{font-size:28px}}</style><main><header><small>HONESTMIX / OFFLINE GUIDE</small><h1>)GUIDE")
        + guideEscape(title) + "</h1><small>" + guideEscape(version) + "</small></header>" + body
        + juce::String::fromUTF8(R"GUIDE(<details><summary>安装、回滚与反馈</summary><p>更新前关闭宿主并保留旧包。便携版请完整解压到新目录；VST3 请保留完整文件夹结构。随包安装 / 回滚工具需要明确目标目录，不要覆盖正在使用的工程或删除宿主扫描配置。</p><p>反馈请提供版本、系统、宿主、采样率、缓冲、复现步骤与错误阶段。连接码、密钥、token、证件、私人工程及完整网络日志不应包含在公开反馈中。诊断包由你检查并主动分享，不自动上传。</p></details><footer>此指南随插件提供，可离线打开。产品仍需真实宿主、声卡及远程协作者验收；不将编译通过等同于所有环境都已通过。</footer></main></html>)GUIDE");
}

inline juce::File writeOfflineGuideTo(const juce::File& directory, const juce::String& product,
                                     const juce::String& version)
{
    if (! directory.createDirectory().wasOk()) return {};
    const auto safeName = (juce::String("HONESTMIX-") + product + "-" + version)
                              .retainCharacters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_");
    const auto file = directory.getChildFile(safeName + "-Guide.html");
    if (! file.replaceWithText(offlineGuideHtml(product, version), false, false, "\n")) return {};
    return file;
}

inline bool openOfflineGuide(const juce::String& product, const juce::String& version)
{
    const auto directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("HONESTMIX").getChildFile("Documentation");
    const auto file = writeOfflineGuideTo(directory, product, version);
    return file.existsAsFile() && juce::URL(file).launchInDefaultBrowser();
}
}}
