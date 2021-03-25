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

.PARAMETER TestReleaseRef
If supplied update the portfile.cmake file's REF and SHA512 with values
associated with the given ref.

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
    [string] $GitHubRepo,

    [string] $TestReleaseRef
)

# If there's nothing in the "port" folder to upload set SkipVcpkgUpdate to true
# and exit. Other steps will check SkipVcpkgUpdate to decide whether to move
# forward.
if (!(Get-ChildItem -Path "$SourceDirectory/port/CONTROL")) {
    Write-Host "###vso[task.setvariable variable=SkipVcpkgUpdate]true"
    exit
}

$tarGzUri = if (-not $TestReleaseRef) {
    Write-Verbose "Initializing Production Release"
    $packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
    "https://github.com/$GitHubRepo/archive/$($packageSpec.packageName).tar.gz" 
} else { 
    Write-Verbose "Initializing Test Release"
    "https://github.com/$GitHubRepo/archive/$TestReleaseRef.tar.gz"         
}

Write-Host "Downloading tarball to compute hash from $tarGzUri" 
$localTarGzPath = New-TemporaryFile
Invoke-WebRequest -Uri $tarGzUri -OutFile $localTarGzPath

$sha512 = (Get-FileHash -Path $localTarGzPath -Algorithm SHA512).Hash.ToLower()
Write-Host "SHA512: $sha512"

Write-Verbose "Writing the SHA512 hash"
$portfileLocation = "$SourceDirectory/port/portfile.cmake"

# Regex replace SHA512 preserving spaces. The placeholder "SHA512 1" is
# recommended in vcpkg documentation
# Before: "   SHA512   1"
# After:  "   SHA512   f6cf1c16c52" 
$portFileContent = Get-Content -Raw -Path $portfileLocation 
$newContent = $portFileContent -replace '(SHA512\s+)1', "`${1}$sha512"

if ($TestReleaseRef) {
    Write-Verbose "Overriding REF with test release ref: $TestReleaseRef"
    $newContent = $newContent -replace '(?m)^(\s+)REF azure.*$', "`${1}REF $TestReleaseRef"
}

$newContent | Set-Content $portfileLocation -NoNewLine
