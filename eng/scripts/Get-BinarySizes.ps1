[CmdletBinding()]
param(
    [Parameter()]
    [string] $BuildDirectory = "$PSScriptRoot../../../build/",

    [Parameter()]
    [string] $ServiceDirectory = "*",

    [Parameter()]
    [switch] $CI = ($null -ne $env:SYSTEM_TEAMPROJECTID)
)

$targetExtension = "lib"
if ($IsLinux -or $IsMacOS) {
    $targetExtension = "a"
}

$binaries = Get-ChildItem `
    -Path "$BuildDirectory/sdk/$ServiceDirectory/*/*.$targetExtension" `

if ($CI) {
    foreach ($binary in $binaries) {
        $metricLogObject = @{
            name = "TestBinarySize";
            value = $binary.Length
            timestamp = (Get-Date -AsUTC).ToString()
            labels = @{ 
                BinaryName = $binary.Name
            }
        }

        $metricLogJson = ConvertTo-Json $metricLogObject -Depth 2 -Compress
        Write-Host "logmetric: $metricLogJson"
    }
    
}
$bins `
    | Format-Table -Property Name, @{Name="SizeInKB"; Expression={"{0:N2}" -f ($_.Length / 1KB)}; Alignment='right'} `
    | Out-String ` 
    | Write-Host 

return $binaries