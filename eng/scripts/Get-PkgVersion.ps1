[CmdletBinding()]
param (

    [Parameter(Mandatory = $true)]
    [string] $ServiceDirectory,
    [Parameter(Mandatory = $true)]
    [string] $PackageName
)

. ${PSScriptRoot}/SdkVersion-Common.ps1

$versionFileLocation = Get-VersionHppLocaiton `
    -ServiceDirectory $ServiceDirectory `
    -PackageName $PackageName

if (!$versionFileLocation) {
    return Get-Content $RepoRoot/sdk/$ServiceDirectory/$PackageName/version.txt
}

$versionFileContents = Get-Content $versionFileLocation -Raw

if (!($versionFileContents -match $VersionRegex)) {
    Write-Error "does not match version information schema"
}

$VersionString = if ($Matches.prerelease) {
    "$($Matches.major).$($Matches.minor).$($Matches.patch)-$($Matches.prerelease)"
} else {
    "$($Matches.major).$($Matches.minor).$($Matches.patch)"
}

return $VersionString