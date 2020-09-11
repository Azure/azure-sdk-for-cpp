<#
.SYNOPSIS
Uploads the release asset and mutates files in $SourceDirectory/port to point to
the uploaded GitHub release asset.

.PARAMETER SourceDirectory
Location of vcpkg assets (usually `<artifact-path>/packages/<package-name>/vcpkg`)

.PARAMETER PackageSpecPath
Location of the relevant package-info.json file

.PARAMETER Username
Username for uploading asset to GitHub release and creating PR (azure-sdk)

.PARAMETER Pass
Password for uploading asset to GitHub release and creating PR

.PARAMETER GitHubRepo
Name of the GitHub repo (of the form Azure/azure-sdk-for-cpp)

#>

param (
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $SourceDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $PackageSpecPath,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Username,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Pass,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $GitHubRepo
)

$packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
$assetFilename = "$($packageSpec.assetName).tar.gz"

# Upload the asset to the release
Write-Verbose "Uploading asset: $assetFilename..."
$assetInfo = & $PSScriptRoot/New-ReleaseAsset.ps1 `
    -ReleaseTag $packageSpec.assetName `
    -AssetPath $SourceDirectory/$assetFilename `
    -GitHubRepo $GitHubRepo `
    -Username $Username `
    -Pass $Pass

Write-Verbose "Mutating files with release info and creating PR"
# Use asset URL to fill in vcpkg port tokens
& $PSScriptRoot/New-VcpkgPortDefinition.ps1 `
    -SourceDirectory "$SourceDirectory/port" `
    -Version $packageSpec.Version `
    -Url $assetInfo.browser_download_url `
    -Filename $assetFilename `
    -Sha512 (vcpkg.exe hash "$SourceDirectory/$assetFilename")
