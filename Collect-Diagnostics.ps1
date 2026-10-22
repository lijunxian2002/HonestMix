param([string]$OutputDirectory = (Join-Path $PSScriptRoot ('diagnostics-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))))
$ErrorActionPreference = 'Stop'
if (Test-Path -LiteralPath $OutputDirectory) { throw 'Choose a new diagnostic output directory; existing files are never overwritten.' }
New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
$artifacts = @()
foreach ($relative in @('client\HONESTMIX Connect.exe','plugin\HONESTMIX Connect.vst3\Contents\x86_64-win\HONESTMIX Connect.vst3','qa\HonestMixTests.exe','client\HONESTMIX Headphone.exe','plugin\HONESTMIX Headphone.vst3\Contents\x86_64-win\HONESTMIX Headphone.vst3','qa\HeadphoneTests.exe','qa\FidelityCheck.exe','qa\ResponseContractTests.exe','qa\TempoRecallTests.exe')) {
    $file = Join-Path $PSScriptRoot $relative
    if (Test-Path -LiteralPath $file -PathType Leaf) {
        $item = Get-Item -LiteralPath $file
        $artifacts += [ordered]@{ path=$relative; bytes=$item.Length; sha256=(Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant(); fileVersion=$item.VersionInfo.FileVersion; productVersion=$item.VersionInfo.ProductVersion }
    }
}
$delivery = $null
$deliveryFile = Join-Path $PSScriptRoot 'evidence\delivery.json'
if (Test-Path -LiteralPath $deliveryFile) {
    $record = Get-Content -LiteralPath $deliveryFile -Raw -Encoding UTF8 | ConvertFrom-Json
    $delivery = [ordered]@{product=$record.product; version=$record.version; createdUtc=$record.createdUtc; sourceArchiveSha256=$record.sourceArchiveSha256}
}
$report = [ordered]@{
    schema='honestmix.customer-diagnostics/1'; createdUtc=(Get-Date).ToUniversalTime().ToString('o')
    osVersion=[Environment]::OSVersion.VersionString; process64Bit=[Environment]::Is64BitProcess; os64Bit=[Environment]::Is64BitOperatingSystem; logicalProcessors=[Environment]::ProcessorCount
    delivery=$delivery; artifacts=$artifacts
    privacy='Local generation only. No computer/user name, serial number, IP, connection code, credential, microphone, audio buffer, user project or configuration file is collected. Nothing is uploaded.'
    limitations='Artifact identity and basic OS facts only; this is not a pass report, latency measurement, public-node probe or DAW compatibility test.'
}
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $OutputDirectory 'diagnostics.json') -Encoding UTF8
@'
请补充后再由你自行分享：
1. 产品版本、独立客户端或宿主名称/版本；不要附用户工程。
2. 触发步骤、预期与实际行为、发生时间和是否能稳定复现。
3. 声卡/驱动类型、双方采样率与宿主块长。不要上传录音或私人音频。
4. Connect：本机或公网节点、Opus/PCM24、哪一方发起；不要填写 HM1 连接码、密钥、令牌、密码。
5. 可另附你检查过的截图和测试报告，先遮住连接码及账户信息。

本文件夹只在本机生成，没有自动联网、上传或发消息。
'@ | Set-Content -LiteralPath (Join-Path $OutputDirectory '复现信息-请填写.txt') -Encoding UTF8
Write-Host "Local diagnostics saved: $OutputDirectory"
