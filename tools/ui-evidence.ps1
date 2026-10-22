# HeadphoneObsidianStars 0.6.1 — offline UI evidence harness.
#
#   * renders the skin with the build's own native vector renderer at 1x/2x/3x/4x
#   * cuts the 200% detail crops out of the NATIVE 4x render (no interpolation)
#   * measures the design reference and emits the pixel/displaysize ratio JSON
#
# Usage: powershell -NoProfile -ExecutionPolicy Bypass -File ui-evidence.ps1 -Exe <path> -Out <dir>
param(
  [Parameter(Mandatory=$true)][string]$Exe,
  [Parameter(Mandatory=$true)][string]$Out,
  [string]$Reference = 'D:\CodexWork\honestmix-headphone-20261016\Assets\HeadphoneObsidianStars.png'
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

function Write-Utf8NoBom([string]$Path, [string]$Text) {
  $enc = New-Object System.Text.UTF8Encoding($false)
  [System.IO.File]::WriteAllText($Path, $Text, $enc)
}
function Get-PngSize([string]$Path) {
  $img = [System.Drawing.Image]::FromFile($Path)
  try { return @{ w = $img.Width; h = $img.Height } } finally { $img.Dispose() }
}
function Save-Png($Bitmap, [string]$Path) {
  $Bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
}
function Write-Sha256([string]$Path) {
  return (Get-FileHash -Algorithm SHA256 -Path $Path).Hash.ToLower()
}

if (-not (Test-Path -LiteralPath $Exe)) { throw "test exe not found: $Exe" }
New-Item -ItemType Directory -Force -Path $Out | Out-Null
$shotsDir = Join-Path $Out 'screens'
New-Item -ItemType Directory -Force -Path $shotsDir | Out-Null

# ── 1. ask the binary for the render plan ────────────────────────────────────
$planRaw = & $Exe --ui-plan
if ($LASTEXITCODE -ne 0) { throw "--ui-plan failed ($LASTEXITCODE)" }
$plan = $planRaw | ConvertFrom-Json
Write-Host ("plan: design {0}x{1}, {2} shots" -f $plan.designWidth, $plan.designHeight, $plan.shots.Count)

$rendered = @()
foreach ($s in $plan.shots) {
  $file = Join-Path $shotsDir $s.file
  if (Test-Path -LiteralPath $file) { Remove-Item -LiteralPath $file -Force }
  $renderJson = & $Exe --ui-render $s.state $s.scale $file
  if ($LASTEXITCODE -ne 0) { throw ("--ui-render {0} x{1} failed: {2}" -f $s.state, $s.scale, $renderJson) }
  $info = $renderJson | ConvertFrom-Json
  $rendered += [pscustomobject]@{
    state  = $s.state
    scale  = [int]$s.scale
    file   = $s.file
    path   = $file
    width  = [int]$info.width
    height = [int]$info.height
    sha256 = (Write-Sha256 $file)
  }
  Write-Host ("rendered {0} x{1} -> {2}x{3}" -f $s.state, $s.scale, $info.width, $info.height)
}

# ── 2. 200 % detail crop, cut from the NATIVE 4x render ─────────────────────
# Source rectangle in logical design units.
$cropLogical = @{ x = 480; y = 30; w = 500; h = 300 }
$cropX = [int]$cropLogical.x * 4
$cropY = [int]$cropLogical.y * 4
$cropW = [int]$cropLogical.w * 4
$cropH = [int]$cropLogical.h * 4
$dstW  = [int]($cropW / 2)
$dstH  = [int]($cropH / 2)
$src4 = Join-Path $shotsDir 'ui_idle_x4.png'
if (-not (Test-Path -LiteralPath $src4)) { throw "native 4x render missing: $src4" }
$src = [System.Drawing.Bitmap]::FromFile($src4)
$cropFile = Join-Path $shotsDir 'ui_idle_200pct_crop_glass_fader.png'
try {
  $rect = New-Object System.Drawing.Rectangle($cropX, $cropY, $cropW, $cropH)
  $crop4x = $src.Clone($rect, $src.PixelFormat)
  try {
    # Decimate by exactly 2 with nearest sampling: every output pixel is one
    # real render pixel, so the crop is a native 200 % render, not an upscale.
    $dst = New-Object System.Drawing.Bitmap($dstW, $dstH, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
      $g = [System.Drawing.Graphics]::FromImage($dst)
      try {
        $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
        $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
        $g.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
        $g.DrawImage($crop4x, (New-Object System.Drawing.Rectangle(0, 0, $dstW, $dstH)))
      } finally { $g.Dispose() }
      Save-Png $dst $cropFile
    } finally { $dst.Dispose() }
  } finally { $crop4x.Dispose() }
} finally { $src.Dispose() }

# ── 3. reference measurement + ratio verdict ────────────────────────────────
$ref = if (Test-Path -LiteralPath $Reference) { Get-PngSize $Reference } else { $null }
$dispW200 = $plan.designWidth * 2
$dispH200 = $plan.designHeight * 2
$scaleX = if ($ref) { [math]::Round($ref.w / $dispW200, 4) } else { $null }
$scaleY = if ($ref) { [math]::Round($ref.h / $dispH200, 4) } else { $null }
$sufficient = ($ref -ne $null) -and ($ref.w -ge $dispW200) -and ($ref.h -ge $dispH200)

$r3 = $rendered | Where-Object { $_.scale -eq 3 -and $_.state -eq 'idle' }
$r4 = $rendered | Where-Object { $_.scale -eq 4 -and $_.state -eq 'idle' }
$r2 = $rendered | Where-Object { $_.scale -eq 2 -and $_.state -eq 'idle' }
$cropSize = Get-PngSize $cropFile

$ratio = [ordered]@{
  schema             = 'headphone-ui-pixel-ratio/1'
  product            = 'HONESTMIX Headphone'
  version            = '0.6.1'
  generatedUtc       = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')
  referenceImage     = [ordered]@{
    path                                       = $Reference
    pixelWidth                                 = if ($ref) { $ref.w } else { $null }
    pixelHeight                                = if ($ref) { $ref.h } else { $null }
    note                                       = 'AI-generated design reference. Used as a造型 reference only; it is never used as a stretched background bitmap.'
  }
  logicalDesign      = [ordered]@{ width = $plan.designWidth; height = $plan.designHeight }
  displayRequirement = [ordered]@{
    scale200Percent                            = 2.0
    requiredDevicePixelWidth                   = $dispW200
    requiredDevicePixelHeight                  = $dispH200
    basis                                      = 'logical design size x 2.0 device scale'
  }
  referenceVsRequirement = [ordered]@{
    referencePixelWidth                        = if ($ref) { $ref.w } else { $null }
    referencePixelHeight                       = if ($ref) { $ref.h } else { $null }
    scaleFactorX                               = $scaleX
    scaleFactorY                               = $scaleY
    covers200Percent                           = $sufficient
    verdict                                    = if ($sufficient) { 'sufficient' } else { 'INSUFFICIENT - reference has fewer pixels than a 200% render needs; stretching it would fabricate detail' }
  }
  decision           = [ordered]@{
    policy                                     = 'vector-native'
    statement                                  = 'The skin is drawn deterministically with JUCE vector primitives (Path / drawText / gradients / per-scale procedural grain) at the current device scale. No bitmap of the reference is stretched or resampled into the UI.'
    referenceUsedFor                           = 'proportions, material layering and contrast only'
  }
  nativeRenders      = [ordered]@{
    oneX                                       = [ordered]@{ width = ($rendered | Where-Object { $_.scale -eq 1 -and $_.state -eq 'idle' }).width; height = ($rendered | Where-Object { $_.scale -eq 1 -and $_.state -eq 'idle' }).height }
    twoX                                       = [ordered]@{ width = $r2.width; height = $r2.height }
    threeX                                     = [ordered]@{ width = $r3.width; height = $r3.height; note = '3x covers 150% display with 2x margin' }
    fourXExport                                = [ordered]@{ width = $r4.width; height = $r4.height; path = $r4.path; sha256 = $r4.sha256; note = 'reusable 4x component asset; 4000x1920 device pixels' }
  }
  detail200PercentCrop = [ordered]@{
    path                                       = $cropFile
    pixelWidth                                 = $cropSize.w
    pixelHeight                                = $cropSize.h
    sourceLogicalRectangle                     = $cropLogical
    method                                     = 'cut from the native 4x render and decimated by exactly 2 with nearest sampling; each output pixel is one real render pixel'
    sha256                                     = (Write-Sha256 $cropFile)
    interpolationUsed                          = $false
  }
  perScaleCoverage   = @($rendered | Where-Object { $_.state -eq 'idle' } | ForEach-Object {
      [ordered]@{ scale = $_.scale; deviceWidth = $_.width; deviceHeight = $_.height
                  coversDisplayPercent = [int]($_.scale * 100); file = $_.file; sha256 = $_.sha256 } })
  allRenders         = @($rendered | ForEach-Object {
      [ordered]@{ state = $_.state; scale = $_.scale; width = $_.width; height = $_.height; file = $_.file; sha256 = $_.sha256 } })
  limitations        = @(
    'Offscreen rendering is not the same as a live Windows display-scaling session: it proves the drawing code is resolution independent, not that every host DPI path is exercised.',
    'The brushed-metal grain tile is procedural and regenerated per integer scale (1x..6x); non-integer device scales reuse the nearest integer tile.'
  )
}

$ratioPath = Join-Path $Out 'ui-pixel-ratio.json'
Write-Utf8NoBom $ratioPath ($ratio | ConvertTo-Json -Depth 8)
Write-Host "wrote $ratioPath"
Write-Host ("reference {0}x{1} vs 200% requirement {2}x{3} -> {4}" -f $ref.w, $ref.h, $dispW200, $dispH200, $ratio.referenceVsRequirement.verdict)
Write-Host "DONE"
