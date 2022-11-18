param(
    [string] $OutFile = "screenshot.bmp"
)

Add-Type -AssemblyName System.Windows.Forms,System.Drawing

$screens = [Windows.Forms.Screen]::AllScreens
$boundaries = [Drawing.Rectangle]::FromLTRB(    
    ($screens.Bounds.Left | Measure-Object -Minimum).Minimum,       # Left
    ($screens.Bounds.Top | Measure-Object -Minimum).Minimum,        # Top 
    ($screens.Bounds.Right | Measure-Object -Maximum).Maximum,      # Right
    ($screens.Bounds.Bottom | Measure-Object -Maximum).Maximum      # Bottom
)

$screenshotBitmap = New-Object System.Drawing.Bitmap(
    [int]$boundaries.width, 
    [int]$boundaries.height
)
[Drawing.Graphics]::FromImage($screenshotBitmap).CopyFromScreen(
    $boundaries.Location,
    [Drawing.Point]::Empty,
    $boundaries.Size
)
$screenshotBitmap.Save($OutFile)
# TODO: Cleanup
