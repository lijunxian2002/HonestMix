# Shared by the explicit user-invoked installer and rollback. No actions on import.
function Get-HMFullPath([string]$Path) {
    $hmFull=[IO.Path]::GetFullPath($Path)
    if ($hmFull -eq [IO.Path]::GetPathRoot($hmFull)) { return $hmFull }
    return $hmFull.TrimEnd([char[]]@(92,47))
}
function Assert-HMBelow([string]$Child,[string]$Parent) {
    $hmChild=Get-HMFullPath $Child; $hmParent=Get-HMFullPath $Parent
    if (-not $hmChild.StartsWith($hmParent+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw "Path is outside intended directory: $Child" }
}
function Assert-HMPlainPath([string]$Path) {
    $hmCurrent=[IO.Path]::GetFullPath($Path)
    while ($hmCurrent) {
        if (Test-Path -LiteralPath $hmCurrent) { if ((Get-Item -LiteralPath $hmCurrent -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Linked paths are not supported: $hmCurrent" } }
        $hmParent=[IO.Path]::GetDirectoryName($hmCurrent)
        if ($hmParent -eq $hmCurrent) { break }; $hmCurrent=$hmParent
    }
}
function Get-HMSnapshot([string]$Root) {
    $hmRoot=Get-HMFullPath $Root; Assert-HMPlainPath $hmRoot
    if (-not (Test-Path -LiteralPath $hmRoot -PathType Container)) { throw "Bundle missing: $Root" }
    $hmPending=New-Object 'System.Collections.Generic.Stack[string]'; $hmPending.Push($hmRoot); $hmFiles=@()
    while ($hmPending.Count) {
        foreach ($hmItem in Get-ChildItem -LiteralPath ($hmPending.Pop()) -Force) {
            if ($hmItem.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Bundle contains a link: $($hmItem.FullName)" }
            if ($hmItem.PSIsContainer) { $hmPending.Push($hmItem.FullName) } else {
                $hmFiles += [PSCustomObject]@{path=$hmItem.FullName.Substring($hmRoot.Length+1).Replace('\','/');bytes=$hmItem.Length;sha256=(Get-FileHash -LiteralPath $hmItem.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
            }
        }
    }
    if (-not $hmFiles.Count) { throw 'Empty plugin bundle.' }
    return @($hmFiles | Sort-Object path)
}
function Assert-HMSnapshot([string]$Root,$Expected) {
    $hmActual=@(Get-HMSnapshot $Root)
    if ((ConvertTo-Json -InputObject $hmActual -Depth 5 -Compress) -ne (ConvertTo-Json -InputObject @($Expected) -Depth 5 -Compress)) { throw "Bundle changed or copy verification failed: $Root" }
}
function Write-HMReceipt([string]$File,$Value) { $Value | ConvertTo-Json -Depth 9 | Set-Content -LiteralPath $File -Encoding UTF8 }
