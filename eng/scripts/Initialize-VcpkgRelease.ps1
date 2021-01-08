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
    [string] $GitHubRepo
)

# If there's nothing in the "port" folder to upload set SkipVcpkgUpdate to true
# and exit. Other steps will check SkipVcpkgUpdate to decide whether to move
# forward.
if (!(Get-ChildItem -Path "$SourceDirectory/port/CONTROL")) {
    Write-Host "###vso[task.setvariable variable=SkipVcpkgUpdate]true"
    exit
}

$packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
$releaseInfoUrl = "https://api.github.com/repos/$GitHubRepo/releases/tags/$($packageSpec.packageName)"

Write-Host "Getting release info"
$githubReleaseInfo = Invoke-RestMethod -Method GET -Uri $releaseInfoUrl
$tarballLocation = New-TemporaryFile

Write-Host "Downloading tarball to compute hash" 
Invoke-WebRequest -Uri $githubReleaseInfo.tarball_url -OutFile $tarballLocation

$sha512 = (Get-FileHash -Path $tarballLocation -Algorithm SHA512).Hash

Write-Verbose "Substituting the SHA512"
$portfileLocation = "$SourceDirectory/port/portfile.cmake"
$newContent = Get-Content -Raw -Path $portfileLocation `
    | ForEach-Object { $_.Replace('%SHA512%', $sha512) }

$newContent | Set-Content $portfileLocation 