# Draws the ASWUnitTests icon (a blue rounded badge with a green check mark and, from 32 px up, a green and red
# progress bar) at each icon size, writes the PNGs and a preview sheet to %TEMP%\asw-check\icon, and with -Ico
# packs them into an .ico (BMP entries below 256 px, PNG at 256, via ImageMagick).
# Usage: & plans\tools\make_icon.ps1 [-Ico <path to .ico>]
param([string]$Ico = '')
Add-Type -AssemblyName System.Drawing
$out = "$env:TEMP\asw-check\icon"
New-Item -ItemType Directory -Force $out | Out-Null

$sizes = 16, 20, 24, 32, 40, 48, 64, 256

function C([string]$hex) { return [System.Drawing.ColorTranslator]::FromHtml($hex) }
$blueTop = C '#2D66B5'
$blueBottom = C '#173B72'
$edge = C '#0E2850'
$green = C '#4BC55E'
$greenDark = C '#2E8F3E'
$red = C '#E5484D'
$track = C '#0E2850'

function RoundedRect([float]$x, [float]$y, [float]$w, [float]$h, [float]$r)
{
    $p = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = 2 * $r
    $p.AddArc($x, $y, $d, $d, 180, 90)
    $p.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $p.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $p.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $p.CloseFigure()
    return $p
}

function Draw([int]$size)
{
    $s = $size / 256.0
    $bmp = New-Object System.Drawing.Bitmap($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = 'AntiAlias'
    $g.PixelOffsetMode = 'HighQuality'
    $g.Clear([System.Drawing.Color]::Transparent)

    $withBar = ($size -ge 32)

    # Badge: the full square at 16-24 px, so the check mark gets every pixel.
    $margin = if ($size -lt 32) { 0 } else { [Math]::Floor(8 * $s) }
    $radius = [Math]::Max(2.5, 44 * $s)
    $side = $size - 2 * $margin
    $badge = RoundedRect $margin $margin $side $side $radius
    $bottom = $margin + $side
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        (New-Object System.Drawing.PointF(0, $margin)), (New-Object System.Drawing.PointF(0, $bottom)),
        $blueTop, $blueBottom)
    $g.FillPath($brush, $badge)
    if ($size -ge 48)
    {
        $pen = New-Object System.Drawing.Pen($edge, [Math]::Max(1, 4 * $s))
        $g.DrawPath($pen, $badge)
    }

    # Check mark: thicker, relative to the size, when it's small.
    if ($withBar)
    {
        $pts = @((64, 120), (108, 164), (194, 76)); $width = 36
    }
    else
    {
        $pts = @((54, 132), (104, 182), (204, 80)); $width = 48
    }
    $points = [System.Drawing.PointF[]]@($pts | ForEach-Object { New-Object System.Drawing.PointF(($_[0] * $s), ($_[1] * $s)) })
    if ($size -ge 48)
    {
        # A darker green rim, for definition against the blue.
        $rim = New-Object System.Drawing.Pen($greenDark, (($width + 8) * $s))
        $rim.StartCap = 'Round'; $rim.EndCap = 'Round'; $rim.LineJoin = 'Round'
        $g.DrawLines($rim, $points)
    }
    $checkPen = New-Object System.Drawing.Pen($green, ($width * $s))
    $checkPen.StartCap = 'Round'; $checkPen.EndCap = 'Round'; $checkPen.LineJoin = 'Round'
    $g.DrawLines($checkPen, $points)

    # Progress bar: mostly green, ending red, as the GUI's shows passes and a failure.
    if ($withBar)
    {
        $bx = 52 * $s; $bw = 152 * $s
        $bh = [Math]::Max(3, 20 * $s); $by = 196 * $s
        $br = $bh / 2
        $g.FillPath((New-Object System.Drawing.SolidBrush($track)), (RoundedRect $bx $by $bw $bh $br))
        $split = $bx + $bw * 0.72
        $g.SetClip((New-Object System.Drawing.RectangleF($bx, $by, ($split - $bx), $bh)))
        $g.FillPath((New-Object System.Drawing.SolidBrush($green)), (RoundedRect $bx $by $bw $bh $br))
        $g.SetClip((New-Object System.Drawing.RectangleF($split, $by, ($bx + $bw - $split), $bh)))
        $g.FillPath((New-Object System.Drawing.SolidBrush($red)), (RoundedRect $bx $by $bw $bh $br))
        $g.ResetClip()
    }

    $g.Dispose()
    $bmp.Save("$out\icon_$size.png", [System.Drawing.Imaging.ImageFormat]::Png)
    return $bmp
}

$images = @{}
foreach ($size in $sizes) { $images[$size] = Draw $size }

# Preview sheet: each size at 1x on a light and a dark band, then 16-48 px magnified 6x on both, then 256 px.
$sheetW = 1400; $sheetH = 700
$sheet = New-Object System.Drawing.Bitmap($sheetW, $sheetH)
$g = [System.Drawing.Graphics]::FromImage($sheet)
$g.Clear((C '#FFFFFF'))
$g.FillRectangle((New-Object System.Drawing.SolidBrush((C '#F3F3F3'))), 0, 0, 700, 400)
$g.FillRectangle((New-Object System.Drawing.SolidBrush((C '#202020'))), 700, 0, 700, 400)
$x = 20
foreach ($size in 16, 20, 24, 32, 40, 48, 64)
{
    $g.DrawImageUnscaled($images[$size], $x, 20)
    $g.DrawImageUnscaled($images[$size], $x + 700, 20)
    $x += $size + 20
}
$g.InterpolationMode = 'NearestNeighbor'
$g.PixelOffsetMode = 'Half'
$x = 20
foreach ($size in 16, 20, 24, 32)
{
    $g.DrawImage($images[$size], $x, 110, $size * 6, $size * 6)
    $g.DrawImage($images[$size], $x + 700, 110, $size * 6, $size * 6)
    $x += $size * 6 + 16
}
$g.InterpolationMode = 'HighQualityBicubic'
$g.DrawImage($images[256], 20, 420, 256, 256)
$g.DrawImage($images[48], 300, 420, 288, 288)
$g.FillRectangle((New-Object System.Drawing.SolidBrush((C '#202020'))), 700, 400, 700, 300)
$g.DrawImage($images[256], 720, 420, 256, 256)
$g.Dispose()
$sheet.Save("$out\preview.png", [System.Drawing.Imaging.ImageFormat]::Png)
"wrote $out"

if ($Ico -ne '')
{
    $pngs = $sizes | ForEach-Object { "$out\icon_$_.png" }
    & magick @pngs $Ico
    & magick identify $Ico
}
