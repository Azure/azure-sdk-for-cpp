<#
.SYNOPSIS
Downloads the release asset and mutates the portfile.cmake file to use the
SHA512 hash of the release asset.

.PARAMETER SourceDirectory
Location of vcpkg assets (usually `<artifact-path>/packages/<package-name>/vcpkg`)

.PARAMETER PackageSpecPath
Location of the relevant package-info.json file

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
$tarGzUri = "https://github.com/$GitHubRepo/archive/$($packageSpec.packageName).tar.gz" 

Write-Host "Downloading tarball to compute hash" 
$localTarGzPath = New-TemporaryFile
Invoke-WebRequest -Uri $tarGzUri -OutFile $localTarGzPath

$sha512 = (Get-FileHash -Path $localTarGzPath -Algorithm SHA512).Hash.ToLower()
Write-Host "SHA512: $sha512"

Write-Verbose "Writing the SHA512 hash"
$portfileLocation = "$SourceDirectory/port/portfile.cmake"
$newContent = Get-Content -Raw -Path $portfileLocation `
    | ForEach-Object { $_.Replace('%SHA512%', $sha512) }

$newContent | Set-Content $portfileLocation 