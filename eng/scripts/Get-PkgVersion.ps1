[CmdletBinding()]
param (

    [Parameter(Mandatory = $true)]
    [string] $ServiceDirectory,
    [Parameter(Mandatory = $true)]
    [string] $PackageName
)

$repoRoot = Resolve-Path "$PSScriptRoot/../..";
. (Join-Path ${repoRoot} "eng" "common" "scripts" "logging.ps1")
. (Join-Path ${repoRoot} "eng" "scripts" "SdkVersion-Common.ps1")

$versionFileLocation = Get-VersionHppLocaiton `
    -ServiceDirectory $ServiceDirectory `
    -PackageName $PackageName

if (!$versionFileLocation) {
    $fallbackpath = Join-Path $RepoRoot "sdk" $ServiceDirectory $PackageName "version.txt"
    if (!(Test-Path $fallbackpath))
    {
        LogWarning "Failed to retrieve package version. No version file found."
        return $null
    }

    $fallback = Get-Content $fallbackpath
    if ($fallback) {
        return $fallback
    } else {
        LogWarning "Cannot locate package version"
        return $null
    }
}

$versionFileContents = Get-Content $versionFileLocation -Raw

if (!($versionFileContents -match $VersionRegex)) {
    LogWarning "does not match version information schema"
    return $null
}

$VersionString = if ($Matches.prerelease) {
    "$($Matches.major).$($Matches.minor).$($Matches.patch)-$($Matches.prerelease)"
} else {
    "$($Matches.major).$($Matches.minor).$($Matches.patch)"
}

return $VersionString
