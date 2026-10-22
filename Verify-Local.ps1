param([string]$OutputDirectory=(Join-Path $PSScriptRoot ('verification-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))))
$ErrorActionPreference='Stop'
if (Test-Path -LiteralPath $OutputDirectory) { throw 'Choose a new verification directory; failed evidence is never overwritten.' }
New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
$hmPackaged=Test-Path -LiteralPath (Join-Path $PSScriptRoot 'qa\HeadphoneTests.exe')
$hmReceipt=[ordered]@{passed=$false;startedUtc=(Get-Date).ToUniversalTime().ToString('o');hardwareAudioOpened=$false;publicNetworkUsed=$false;scope='Native offline DSP/UI, VST3 scan/process, exact response/tempo contracts and absolute SRC fidelity. No standalone audio device, third-party DAW, microphone or hardware latency test.';binaries=@();runs=@()}
try {
    if ($hmPackaged) {
        $hmDelivery=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'evidence\delivery.json') -Raw -Encoding UTF8 | ConvertFrom-Json
        $hmVersion=$hmDelivery.version; $hmQa=Join-Path $PSScriptRoot 'qa'; $hmPlugin=Join-Path $PSScriptRoot 'plugin\HONESTMIX Headphone.vst3'
    } else {
        $hmCmake=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'CMakeLists.txt') -Raw
        if ($hmCmake -notmatch 'project\s*\(\s*HonestMixHeadphone\s+VERSION\s+(\d+\.\d+\.\d+)') { throw 'Project version missing' }
        $hmVersion=$Matches[1]; $hmPlugin=Join-Path $PSScriptRoot 'build\HonestMixHeadphone_artefacts\Release\VST3\HONESTMIX Headphone.vst3'
    }
    $hmReceipt.version=$hmVersion
    $hmClient=if($hmPackaged){Join-Path $PSScriptRoot 'client\HONESTMIX Headphone.exe'}else{Join-Path $PSScriptRoot 'build\HonestMixHeadphone_artefacts\Release\Standalone\HONESTMIX Headphone.exe'}
    $hmPluginBinary=Join-Path $hmPlugin 'Contents\x86_64-win\HONESTMIX Headphone.vst3'
    foreach($hmArtifact in @(@{name='Standalone';path=$hmClient;expected=if($hmPackaged){$hmDelivery.exeSha256}else{$null}},@{name='VST3';path=$hmPluginBinary;expected=if($hmPackaged){$hmDelivery.vst3Sha256}else{$null}})) {
        $hmInfo=(Get-Item -LiteralPath $hmArtifact.path).VersionInfo
        $hmSha=(Get-FileHash -LiteralPath $hmArtifact.path -Algorithm SHA256).Hash.ToLowerInvariant()
        if($hmInfo.FileVersion -ne $hmVersion -or $hmInfo.ProductVersion -ne $hmVersion -or ($hmPackaged -and $hmSha -ne $hmArtifact.expected)) { throw "Product version/hash mismatch: $($hmArtifact.name)" }
        $hmReceipt.binaries+=@{name=$hmArtifact.name;sha256=$hmSha;version=$hmVersion}
    }
    if(-not $hmPackaged) {
        $hmProvenanceReport=Join-Path $OutputDirectory 'profile-generation.json'
        & node (Join-Path $PSScriptRoot 'tools\profile-generation\verify.mjs') $hmProvenanceReport 2>&1 | Tee-Object -FilePath (Join-Path $OutputDirectory 'profile-generation.log')
        if($LASTEXITCODE -ne 0) { throw 'Pinned profile regeneration failed; retain first report.' }
        $hmProvenance=Get-Content -LiteralPath $hmProvenanceReport -Raw -Encoding UTF8 | ConvertFrom-Json
        if($hmProvenance.passed -ne $true -or $hmProvenance.version -ne $hmVersion) { throw 'Invalid profile generation proof' }
        $hmReceipt.profileGeneration=$hmProvenance
    }
    foreach ($hmName in @('HeadphoneTests','FidelityCheck','ResponseContractTests','TempoRecallTests')) {
        $hmExe=if($hmPackaged){Join-Path $hmQa ($hmName+'.exe')}else{Join-Path $PSScriptRoot ('build\'+$hmName+'_artefacts\Release\'+$hmName+'.exe')}
        $hmInfo=(Get-Item -LiteralPath $hmExe).VersionInfo
        if ($hmInfo.FileVersion -ne $hmVersion -or $hmInfo.ProductVersion -ne $hmVersion) { throw "Wrong QA version: $hmName" }
        $hmSha=(Get-FileHash -LiteralPath $hmExe -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($hmPackaged) {
            $hmExpected=if($hmName -eq 'HeadphoneTests'){$hmDelivery.testExeSha256}else{@($hmDelivery.requiredQaBinaries | Where-Object path -eq ($hmName+'.exe'))[0].sha256}
            if ($hmSha -ne $hmExpected) { throw "QA hash differs from delivery record: $hmName" }
        }
        $hmReceipt.binaries+=@{name=$hmName;sha256=$hmSha;version=$hmVersion}
        $hmReport=if($hmName -eq 'FidelityCheck'){Join-Path $OutputDirectory 'fidelity-check.json'}elseif($hmName -eq 'ResponseContractTests'){Join-Path $OutputDirectory 'response-contract.json'}elseif($hmName -eq 'TempoRecallTests'){Join-Path $OutputDirectory 'tempo-recall\result.json'}else{Join-Path $OutputDirectory 'headphone-tests.json'}
        # Keep even a single report path as an array: Windows PowerShell 5.1
        # splats a scalar string into individual characters for a native process.
        $hmArguments=@($hmReport)
        if($hmName -eq 'HeadphoneTests') { $hmArguments=@($OutputDirectory,$hmPlugin) }
        if($hmName -eq 'TempoRecallTests') { $hmArguments=@((Join-Path $OutputDirectory 'tempo-recall')) }
        & $hmExe @hmArguments 2>&1 | Tee-Object -FilePath (Join-Path $OutputDirectory ($hmName+'.log'))
        $hmExit=$LASTEXITCODE; $hmReceipt.runs+=@{name=$hmName;executable=$hmExe;arguments=$hmArguments;exitCode=$hmExit;report=(Split-Path -Leaf $hmReport)}
        if ($hmExit -ne 0) { throw "$hmName failed; retain this directory before another run." }
        $hmResult=Get-Content -LiteralPath $hmReport -Raw -Encoding UTF8 | ConvertFrom-Json
        if ($hmResult.passed -ne $true -or $hmResult.hardwareAudioOpened -ne $false) { throw "Invalid test result: $hmName" }
        if ($hmResult.version -and $hmResult.version -ne $hmVersion) { throw "Report version mismatch: $hmName" }
    }
    $hmReceipt.passed=$true; Write-Host "Offline verification passed. Evidence: $OutputDirectory"
} catch { $hmReceipt.error=$_.Exception.Message; throw } finally {
    $hmReceipt.finishedUtc=(Get-Date).ToUniversalTime().ToString('o')
    $hmReceipt | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $OutputDirectory 'verification.json') -Encoding UTF8
}
