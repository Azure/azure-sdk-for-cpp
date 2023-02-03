[CmdletBinding()]
param(
    [Parameter()]
    [string] $BuildDirectory = "$PSScriptRoot/../../build/",

    [Parameter()]
    [string] $ServiceDirectory = "*",

    [Parameter()]
    [string] $OsVMImage,

    [Parameter()]
    [string] $CmakeArgs,

    [Parameter()]
    [string] $CmakeEnvArg,

    [Parameter()]
    [string] $BuildArgs,

    [Parameter()]
	[string] $VcpkgArgs,

    [Parameter()]
    [string] $Job,

    [Parameter()]
    [string] $BuildReason,

    [Parameter()]
    [string] $SourceBranch,

    [Parameter()]
    [switch] $CI = ($null -ne $env:SYSTEM_TEAMPROJECTID),

    [Parameter()]
    [hashtable]$ExtraLabels = @{}
)

. $PSScriptRoot/../common/scripts/common.ps1

function setEnvVar($key, $value) {
    Write-Host "##vso[task.setvariable variable=_$key;issecret=true;]$value"
}

function getTargetOs {
    if ($OsVMImage -like 'macOS*') {
        return $OsVMImage
    }

    if ($OsVMImage -match "^MMS2019$|^win-2019$") {
        return "win-2019"
    }

    if ($OsVMImage -match "^MMS2022$|^win-2022$|^windows-2022$") {
        return "win-2022"
    }

    if ($OsVMImage -eq "MMSUbuntu18.04") {
        return "ubuntu-18.04"
    }

    if ($OsVMImage -eq "MMSUbuntu20.04") {
        return "ubuntu-20.04"
    }

    LogError "Could not infer target OS from " $OSVmImage
}

function getTargetArchitecture {
    if ($OSVmImage -like 'macOS*') {
        return "x64"
    }

    if ($env:CMAKE_GENERATOR_PLATFORM -and $env:CMAKE_GENERATOR_PLATFORM -eq "Win32") {
        return "x86"
    }

    if ($env:CMAKE_GENERATOR_PLATFORM -and $env:CMAKE_GENERATOR_PLATFORM -eq "x64") {
        return "x64"
    }

    # Most builds target x64 by default
    return "x64"
}

function getToolChain {
    if ($OSVmImage -like 'macOS*') {
        return "AppleClang 12"
    }

    if ($OSVmImage -match "MMS\d{4}") {
        return "MSVC"
    }

	if ($OSVmImage -match "windows-\d{4}") {
        return "MSVC17"
    }

    if ($OSVmImage.Contains("Ubuntu")) {
        if ($CmakeEnvArg.Contains('g++-5')) {
            return 'g++-5'
        } elseif ($env:CXX -and $env:CXX.Contains("g++8")) {
            return 'g++-8'
        } elseif ($env:CXX -and $env:CXX.Contains("g++-9")) {
            return 'g++-9'
        } elseif ($env:CXX -and $env:CXX.Contains("clang-11")) {
            return 'clang-11'
        }
        return "g++-7"
    }
    LogError "Could not infer toolchain from " $OSVmImage and $CmakeEnvArg
}

function getTargetPlatform {
    if ($OSVmImage -like 'macOS*') {
        return "macos"
    }

    if ($OSVmImage -match 'MMS\d{4}' -or $OsVMImage -match "windows-\d{4}") {
        if (!$env:CMAKE_SYSTEM_NAME -and !$CmakeArgs.Contains('WindowsStore')) {
            return 'win32'
        } elseif ($env:CMAKE_SYSTEM_NAME -eq 'WindowsStore' -or $CmakeArgs.Contains('WindowsStore')) {
            return 'uwp'
        }
    }

    if ($OSVmImage.Contains("Ubuntu")) {
        return 'linux'
    }

    LogError "Could not infer target platform from " $OSVmImage
}

function getBuildConfiguration {
    if ($env:CMAKE_BUILD_TYPE) {
        return $env:CMAKE_BUILD_TYPE
    }

    if ($BuildArgs.Contains("--config Debug")) {
        return "Debug"
    }

    if ($BuildArgs.Contains("--config Release")) {
        return "Release"
    }

    # Most builds default to Debug unless overridden by configuration variables
    return "Debug"
}

$searchPath = "$BuildDirectory/sdk/$ServiceDirectory/*/*.a"
if ($IsWindows) {
    $searchPath = "$BuildDirectory/sdk/$ServiceDirectory/*/*/*.lib"
}

$binaries = Get-ChildItem -Path $searchPath

$buildLabels = @{
    TargetOs = getTargetOs;
    Toolchain = getToolChain;
    BuildConfig = getBuildConfiguration;
    TargetArchitecture = getTargetArchitecture;
    TargetPlatform = getTargetPlatform;
}

if ($CI) {
    foreach ($binary in $binaries) {
        $metricLogObject = @{
            name = "BinarySize";
            value = $binary.Length
            timestamp = (Get-Date -AsUTC).ToString("o")
            labels = @{
                Job = $Job;
                BuildReason = $BuildReason;
                SourceBranch = $SourceBranch;
                BinaryName = $binary.Name;
            } + $buildLabels + $ExtraLabels
        }

        $metricLogJson = ConvertTo-Json $metricLogObject -Compress
        Write-Host "logmetric: $metricLogJson"
    }
}
$binaries `
    | Format-Table -Property Name, @{Name="SizeInKB"; Expression={"{0:N2}" -f ($_.Length / 1KB)}; Alignment='right'} `
    | Out-String `
    | Write-Host

return $binaries
