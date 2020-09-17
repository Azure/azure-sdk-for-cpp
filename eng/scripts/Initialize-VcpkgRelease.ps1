<#
.SYNOPSIS
Uploads the release asset and mutates files in $SourceDirectory/port to point to
the uploaded GitHub release asset.

.PARAMETER SourceDirectory
Location of vcpkg assets (usually `<artifact-path>/packages/<package-name>/vcpkg`)

.PARAMETER PackageSpecPath
Location of the relevant package-info.json file

.PARAMETER GitHubPat
PAT for uploading asset to GitHub release and creating PR

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
    [string] $GitHubPat,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $GitHubRepo
)

# If there's nothing in the "port" folder to upload set SkipVcpkgUpdate to true
# and exit. Other steps will check SkipVcpkgUpdate to decide whether to move
# forward.
if (!(Get-ChildItem -Path "$SourceDirectory/port")) {
    Write-Host "###vso[task.setvariable variable=SkipVcpkgUpdate]true"
    exit
}

$packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
$assetFilename = "$($packageSpec.packageName).tar.gz"

# Upload the asset to the release
Write-Verbose "Uploading asset: $assetFilename..."
$assetInfo = & $PSScriptRoot/../common/scripts/New-ReleaseAsset.ps1 `
    -ReleaseTag $packageSpec.packageName `
    -AssetPath $SourceDirectory/$assetFilename `
    -GitHubRepo $GitHubRepo `
    -GitHubPat $GitHubPat

$sha512 = (Get-FileHash -Path "$SourceDirectory/$assetFilename" -Algorithm SHA512).Hash

Write-Verbose "Mutating files with release info and creating PR"
# Use asset URL to fill in vcpkg port tokens
& $PSScriptRoot/New-VcpkgPortDefinition.ps1 `
    -SourceDirectory "$SourceDirectory/port" `
    -Version $packageSpec.Version `
    -Url $assetInfo.browser_download_url `
    -Filename $assetFilename `
    -Sha512 $sha512
