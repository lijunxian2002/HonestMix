param(
    [Parameter(Mandatory=$true)][string]$DestinationRoot,
    [string]$BackupRoot=(Join-Path $env:LOCALAPPDATA 'HONESTMIX\plugin-backups'),
    [switch]$HostClosed
)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'VST3-Delivery.ps1')
if (-not $HostClosed) { throw 'Save projects and close the DAW, then invoke with -HostClosed. This script never controls a DAW.' }
$hmDestination=Get-HMFullPath $DestinationRoot; $hmBackupRoot=Get-HMFullPath $BackupRoot
if ($hmDestination -eq [IO.Path]::GetPathRoot($hmDestination) -or $hmBackupRoot -eq [IO.Path]::GetPathRoot($hmBackupRoot)) { throw 'Choose named install and backup directories, not a drive root.' }
Assert-HMPlainPath $hmDestination; Assert-HMPlainPath $hmBackupRoot
if ([IO.Path]::GetPathRoot($hmDestination) -ne [IO.Path]::GetPathRoot($hmBackupRoot)) { throw 'BackupRoot must be on the same volume and outside the plugin scan directory. Supply -BackupRoot explicitly for another drive.' }
if ($hmDestination -eq (Get-HMFullPath $PSScriptRoot) -or $hmDestination.StartsWith((Get-HMFullPath $PSScriptRoot)+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Choose a destination outside the delivery package.' }
if ($hmBackupRoot -eq $hmDestination -or $hmBackupRoot.StartsWith($hmDestination+'\',[StringComparison]::OrdinalIgnoreCase) -or $hmDestination.StartsWith($hmBackupRoot+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'BackupRoot and DestinationRoot must be separate; backups must not be scanned as plugins.' }
$hmBundles=@(Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'plugin') -Directory -Filter '*.vst3')
if ($hmBundles.Count -ne 1) { throw 'Expected one complete VST3 bundle in this package.' }
$hmSource=$hmBundles[0].FullName; $hmTarget=Join-Path $hmDestination $hmBundles[0].Name
if ((Get-HMFullPath $hmSource) -eq (Get-HMFullPath $hmTarget)) { throw 'Source and destination are identical.' }
$hmFiles=@(Get-HMSnapshot $hmSource)
$hmManifest=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'SHA256.json') -Raw -Encoding UTF8 | ConvertFrom-Json
if ($hmManifest.files) { $hmManifest=$hmManifest.files }
$hmPrefix='plugin/'+$hmBundles[0].Name+'/'
$hmExpected=@($hmManifest | Where-Object { $_.path.StartsWith($hmPrefix,[StringComparison]::OrdinalIgnoreCase) })
if ($hmExpected.Count -ne $hmFiles.Count) { throw 'Plugin files do not match the delivery manifest.' }
foreach ($hmFile in $hmFiles) {
    $hmListed=@($hmExpected | Where-Object path -eq ($hmPrefix+$hmFile.path))
    if ($hmListed.Count -ne 1 -or $hmListed[0].sha256 -ne $hmFile.sha256 -or $hmListed[0].bytes -ne $hmFile.bytes) { throw "Plugin checksum mismatch: $($hmFile.path)" }
}
New-Item -ItemType Directory -Path $hmDestination -Force | Out-Null
New-Item -ItemType Directory -Path $hmBackupRoot -Force | Out-Null
$hmToken=[Guid]::NewGuid().ToString('N'); $hmOperation=Join-Path $hmBackupRoot ('install-'+$hmToken); $hmStageRoot=Join-Path $hmDestination ('.honestmix-stage-'+$hmToken)
Assert-HMBelow $hmOperation $hmBackupRoot; Assert-HMBelow $hmStageRoot $hmDestination; Assert-HMBelow $hmTarget $hmDestination
New-Item -ItemType Directory -Path $hmOperation | Out-Null; New-Item -ItemType Directory -Path $hmStageRoot | Out-Null
Write-HMReceipt (Join-Path $hmOperation 'owner.json') @{token=$hmToken;purpose='Explicit HONESTMIX VST3 install backup'}
Write-HMReceipt (Join-Path $hmStageRoot 'owner.json') @{token=$hmToken;purpose='New bundle verified before rename; no automatic deletion'}
$hmStage=Join-Path $hmStageRoot $hmBundles[0].Name; $hmPrevious=Join-Path $hmOperation 'previous.vst3'; $hmReceiptFile=Join-Path $hmOperation 'install-receipt.json'
$hmReceipt=[ordered]@{schema='honestmix.vst3-install/1';token=$hmToken;createdUtc=(Get-Date).ToUniversalTime().ToString('o');destinationRoot=$hmDestination;backupRoot=$hmBackupRoot;target=$hmTarget;previous=$hmPrevious;hadPrevious=(Test-Path -LiteralPath $hmTarget);previousFiles=@();installedFiles=$hmFiles;status='staging';rolledBack=$false}
try {
    Copy-Item -LiteralPath $hmSource -Destination $hmStage -Recurse
    Assert-HMSnapshot $hmStage $hmFiles
    if ($hmReceipt.hadPrevious) {
        $hmReceipt.previousFiles=@(Get-HMSnapshot $hmTarget)
        Assert-HMBelow $hmTarget $hmDestination; Assert-HMBelow $hmPrevious $hmOperation; Assert-HMPlainPath $hmTarget
        Move-Item -LiteralPath $hmTarget -Destination $hmPrevious
        Assert-HMSnapshot $hmPrevious $hmReceipt.previousFiles
    }
    Assert-HMBelow $hmStage $hmDestination; Assert-HMBelow $hmTarget $hmDestination
    if (Test-Path -LiteralPath $hmTarget) { throw 'Destination appeared during installation; refusing overwrite.' }
    Move-Item -LiteralPath $hmStage -Destination $hmTarget
    Assert-HMSnapshot $hmTarget $hmFiles
    $hmReceipt.status='installed'; Write-HMReceipt $hmReceiptFile $hmReceipt
    Write-Host "Installed complete bundle: $hmTarget"
    Write-Host "Verified backup/rollback receipt: $hmReceiptFile"
    Write-Host 'No DAW configuration or global scan setting was changed. Rescan this folder manually in your host.'
} catch {
    $hmReceipt.status='failed'; $hmReceipt.error=$_.Exception.Message
    if ($hmReceipt.hadPrevious -and (Test-Path -LiteralPath $hmPrevious) -and -not (Test-Path -LiteralPath $hmTarget)) {
        Assert-HMBelow $hmPrevious $hmOperation; Assert-HMBelow $hmTarget $hmDestination
        Move-Item -LiteralPath $hmPrevious -Destination $hmTarget
        $hmReceipt.restoredAfterFailure=$true
    }
    Write-HMReceipt $hmReceiptFile $hmReceipt; throw
}
