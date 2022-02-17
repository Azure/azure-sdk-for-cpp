[CmdletBinding()]
param(
    [Parameter()]
    [string] $BuildDirectory = "$PSScriptRoot../../../build/",

    [Parameter()]
    [string] $ServiceDirectory = "*",

    [Parameter()]
    [switch] $CI = ($null -ne $env:SYSTEM_TEAMPROJECTID),

    [Parameter()]
    [hashtable]$ExtraLabels = @{}
)

$searchPath = "$BuildDirectory/sdk/$ServiceDirectory/*/*.a"
if ($IsWindows) {
    $searchPath = "$BuildDirectory/sdk/$ServiceDirectory/*/*/*.lib"
}

$binaries = Get-ChildItem -Path $searchPath

if ($CI) {
    foreach ($binary in $binaries) {
        $metricLogObject = @{
            name = "BinarySize";
            value = $binary.Length
            timestamp = (Get-Date -AsUTC).ToString()
            labels = @{
                BinaryName = $binary.Name;
            } + $ExtraLabels
        }

        $metricLogJson = ConvertTo-Json $metricLogObject -Depth 2 -Compress
        Write-Host "logmetric: $metricLogJson"
    }
}
$binaries `
    | Format-Table -Property Name, @{Name="SizeInKB"; Expression={"{0:N2}" -f ($_.Length / 1KB)}; Alignment='right'} `
    | Out-String `
    | Write-Host 

return $binaries
