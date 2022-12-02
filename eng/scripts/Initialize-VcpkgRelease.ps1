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

.PARAMETER DailyReleaseRef
If supplied update the portfile.cmake file's REF and SHA512 with values
associated with the given ref.

#>
[CmdletBinding()]
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

    [string] $DailyReleaseRef
)

# If there's nothing in the "port" folder to upload set SkipVcpkgUpdate to true
# and exit. Other steps will check SkipVcpkgUpdate to decide whether to move
# forward.
if (!(Get-ChildItem -Path "$SourceDirectory/port/vcpkg.json")) {
    Write-Host "###vso[task.setvariable variable=SkipVcpkgUpdate]true"
    exit
}

$packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
$tarGzUri = "https://github.com/$GitHubRepo/archive/$($packageSpec.packageName).tar.gz" 

if ($DailyReleaseRef) {
    Write-Verbose "Initializing Daily Release"
    $tarGzUri =  "https://github.com/$GitHubRepo/archive/$DailyReleaseRef.tar.gz"    
}

Write-Host "Downloading tarball to compute hash from $tarGzUri" 
$localTarGzPath = New-TemporaryFile
Invoke-WebRequest -Uri $tarGzUri -OutFile $localTarGzPath

$sha512 = (Get-FileHash -Path $localTarGzPath -Algorithm SHA512).Hash.ToLower()
Write-Host "SHA512: $sha512"

Write-Verbose "Writing the SHA512 hash"
$portfileLocation = "$SourceDirectory/port/portfile.cmake"

# Regex replace SHA512 preserving spaces. The placeholder "SHA512 0" is
# recommended in vcpkg documentation
# Before: "   SHA512   0"
# After:  "   SHA512   f6cf1c16c52" 
$portFileContent = Get-Content -Raw -Path $portfileLocation 
$newContent = $portFileContent -replace '(SHA512\s+)0', "`${1}$sha512"

if ($DailyReleaseRef) {
    Write-Verbose "Overriding REF with test release ref: $DailyReleaseRef"
    $newContent = $newContent -replace '(?m)^(\s+)REF azure.*$', "`${1}REF $DailyReleaseRef"
}

$newContent | Set-Content $portfileLocation -NoNewLine