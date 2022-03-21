param(
    [string] $VcpkgBetaFolder,
    [string] $VcpkgFolder,
    [string] $ReleaseArtifactSourceDirectory,
    [string] $VcpkgPortName,
    [string] $GitCommitParameters,
    [string] $BuildIdentifier = $env:BUILD_BUILDID
)

."$PSSCriptRoot/../common/scripts/common.ps1"
Set-StrictMode -Version 3

# To ensure a clean synchronization remove all files at the destination.
# This ensures that files no longer present in the build output do not
# persist in later versions.
$portFolder = "$VcpkgBetaFolder/ports/$VcpkgPortName"
if (Test-Path $portFolder) {
    Remove-Item $portFolder -Recurse -Force
}
New-Item -ItemType Directory -Path $portFolder

Copy-Item `
    -Path "$ReleaseArtifactSourceDirectory/vcpkg/port/*" `
    -Destination $portFolder

$rawPackageInfo = Get-Content -Raw -Path $ReleaseArtifactSourceDirectory/package-info.json
$packageInfo = ConvertFrom-Json $rawPackageInfo

$originalLocation = Get-Location
try {
    Set-Location $VcpkgFolder
    if ($IsWindows) {
        Write-Host "./bootstrap-vcpkg.bat"
        ./bootstrap-vcpkg.bat
    } else {
        Write-Host "./bootstrap-vcpkg.sh"
        ./bootstrap-vcpkg.sh
    }

    Set-Location $VcpkgBetaFolder

    Write-Host "$VcpkgFolder/vcpkg format-manifest --all --vcpkg-root=. --x-scripts-root=$VcpkgFolder/scripts"
    & $VcpkgFolder/vcpkg format-manifest `
        --all `
        --vcpkg-root=. `
        --x-scripts-root=$VcpkgFolder/scripts  # This param is extra

    Write-Host "git add -A"
    git add -A
    Write-Host "git $GitCommitParameters commit -m `"$(Get-Date -Format "yyyy-MM-dd" ): $VcpkgPortName $($packageInfo.version)`""
    "git $GitCommitParameters commit -m '$(Get-Date -Format "yyyy-MM-dd" ): $VcpkgPortName $($packageInfo.version)'"

    Write-Host "$VcpkgFolder/vcpkg x-add-version $VcpkgPortName --vcpkg-root=. --x-scripts-root=$VcpkgFolder/scripts"
    & $VcpkgFolder/vcpkg x-add-version `
        $VcpkgPortName `
        --vcpkg-root=. `
        --x-scripts-root=$VcpkgFolder/scripts  # This param is extra

    $tagName = "$(Get-Date -Format "yyyyMMdd" ).$($BuildIdentifier)_$($VcpkgPortName)_$($packageInfo.version)"

    # Update vcpkg-configuration.json to include this package and set the
    # baseline
    $vcpkgConfigPath = "$VcpkgBetaFolder/vcpkg-configuration.json"
    $rawVcpkgConfig = Get-Content -Raw -Path $vcpkgConfigPath
    $vcpkgConfig = ConvertFrom-Json $rawVcpkgConfig

    $vcpkgConfig.registries[0].baseline = $tagName
    if (!($vcpkgConfig.registries[0].packages -contains $VcpkgPortName)) {
        $vcpkgConfig.registries[0].packages += $VcpkgPortName
    }

    $vcpkgConfigJson = ConvertTo-Json $vcpkgConfig -Depth 100
    Set-Content -Path $vcpkgConfigPath -Value $vcpkgConfigJson

    Write-Host "git add -A"
    git add -A
    Write-Host "git $GitCommitParameters commit --amend --no-edit"
    "git $GitCommitParameters commit --amend --no-edit"

    Write-Host "git tag $tagName"
    git tag $tagName

    # Validate overlay port installs (may only be possible after a push)
    Write-Host "$VcpkgFolder/vcpkg" install $VcpkgPortName --overlay-ports=$VcpkgBetaFolder
    ."$VcpkgFolder/vcpkg" install $VcpkgPortName --overlay-ports=$VcpkgBetaFolder

    if ($LASTEXITCODE) {
        LogError "Port validation failed. Ensure the port builds properly"
        exit 1
    }
} finally {
    Set-Location $originalLocation
}
