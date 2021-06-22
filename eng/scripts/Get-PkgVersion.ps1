[CmdletBinding()]
param (

    [Parameter(Mandatory = $true)]
    [string] $ServiceDirectory,
    [Parameter(Mandatory = $true)]
    [string] $PackageName
)

. (Join-Path $PSScriptRoot ".." common scripts common.ps1)

$versionFileLocation = Get-VersionHppLocation `
    -ServiceDirectory $ServiceDirectory `
    -PackageName $PackageName

if (!$versionFileLocation) {
    $fallbackpath = Join-Path $RepoRoot sdk $ServiceDirectory $PackageName version.txt
    if (!(Test-Path $fallbackpath))
    {
        LogWarning "Failed to retrieve package version for '$PackageName'. No version file found."
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
