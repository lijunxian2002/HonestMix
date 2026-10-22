param([Parameter(Mandatory=$true)][string]$Receipt,[switch]$HostClosed)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'VST3-Delivery.ps1')
if (-not $HostClosed) { throw 'Save projects and close the DAW, then invoke with -HostClosed. This script never controls a DAW.' }
Assert-HMPlainPath $Receipt
$hmReceiptFile=(Resolve-Path -LiteralPath $Receipt).Path; $hmOperation=Split-Path -Parent $hmReceiptFile
$hmRecord=Get-Content -LiteralPath $hmReceiptFile -Raw -Encoding UTF8 | ConvertFrom-Json
if ($hmRecord.schema -ne 'honestmix.vst3-install/1' -or $hmRecord.status -ne 'installed' -or $hmRecord.rolledBack) { throw 'Receipt is not an active completed installation.' }
$hmOwner=Get-Content -LiteralPath (Join-Path $hmOperation 'owner.json') -Raw | ConvertFrom-Json
if ($hmOwner.token -ne $hmRecord.token -or (Split-Path -Leaf $hmOperation) -ne ('install-'+$hmRecord.token)) { throw 'Backup owner token does not match.' }
if (-not $hmRecord.target.EndsWith('.vst3',[StringComparison]::OrdinalIgnoreCase) -or [IO.Path]::GetPathRoot($hmRecord.target) -ne [IO.Path]::GetPathRoot($hmOperation)) { throw 'Invalid VST3 target or cross-volume rollback receipt.' }
Assert-HMBelow $hmOperation $hmRecord.backupRoot; Assert-HMBelow $hmRecord.target $hmRecord.destinationRoot; Assert-HMBelow $hmRecord.previous $hmOperation
Assert-HMPlainPath $hmRecord.target; Assert-HMPlainPath $hmRecord.previous
Assert-HMSnapshot $hmRecord.target $hmRecord.installedFiles
if ($hmRecord.hadPrevious) { Assert-HMSnapshot $hmRecord.previous $hmRecord.previousFiles }
$hmArchived=Join-Path $hmOperation 'replaced-new-on-rollback.vst3'
Assert-HMBelow $hmArchived $hmOperation
if (Test-Path -LiteralPath $hmArchived) { throw 'Rollback archive already exists; refusing overwrite.' }
Move-Item -LiteralPath $hmRecord.target -Destination $hmArchived
if ($hmRecord.hadPrevious) {
    Assert-HMBelow $hmRecord.previous $hmOperation; Assert-HMBelow $hmRecord.target $hmRecord.destinationRoot
    Move-Item -LiteralPath $hmRecord.previous -Destination $hmRecord.target
    Assert-HMSnapshot $hmRecord.target $hmRecord.previousFiles
}
$hmRecord.rolledBack=$true; $hmRecord.status='rolled_back'
Write-HMReceipt $hmReceiptFile $hmRecord
Write-Host "Rollback complete. New bundle retained outside scan folder: $hmArchived"
Write-Host 'No plugin files were deleted. Rescan in your host manually.'
