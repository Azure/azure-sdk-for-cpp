param(
    [string] $ReleaseArtifactSourceDirectory,
    [string] $VcpkgFolder,
    [string] $VcpkgPortName
)

."$PSSCriptRoot/../common/scripts/common.ps1"

Set-StrictMode -Version 3

$packageJsonContents = Get-Content `
    -Path "$ReleaseArtifactSourceDirectory/package-info.json" `
    -Raw
$packageJson = ConvertFrom-Json $packageJsonContents
$packageVersionSemver = [AzureEngSemanticVersion]::ParseVersionString($packageJson.version)

if (!$packageVersionSemver.IsPrerelease) {
    Write-Host "Package version is GA ($($packageJson.version)), publish to vcpkg"
    Write-Host "##vso[task.setvariable variable=PublishToVcpkg]true"
    exit 0
}
Write-Host "Released package is preview"

# The package does not exist in vcpkg, publish to vcpkg
$vcpkgJsonPath = "$VcpkgFolder/ports/$VcpkgPortName/vcpkg.json"
if (!(Test-Path $vcpkgJsonPath)) {
    Write-Host "Package ($VcpkgPortName) has not been published, publish to vcpkg"
    Write-Host "##vso[task.setvariable variable=PublishToVcpkg]true"
    exit 0
}
Write-Host "Package has been published before"

$vcpkgJsonContents = Get-Content -Raw -Path $vcpkgJsonPath
$vcpkgJson = ConvertFrom-Json $vcpkgJsonContents

$existingVersion = [AzureEngSemanticVersion]::ParseVersionString($vcpkgJson.'version-semver')

# Published version is a prerelease
if ($existingVersion.IsPrerelease) {
    Write-Host "Existing version ($($vcpkgJson.'version-semver')) is prerelease, publish to vcpkg"
    Write-Host "##vso[task.setvariable variable=PublishToVcpkg]true"
    exit 0
}
Write-Host "Existing version is GA"

Write-Host "Criteria for publishing not satisifed, do NOT publish to vcpkg"
Write-Host "##vso[task.setvariable variable=PublishToVcpkg]false"