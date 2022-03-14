param(
    [string] $VcpkgBetaFolder,
    [string] $VcpkgFolder,
    [string] $ReleaseArtifactSourceDirectory,
    [string] $VcpkgPortName,
    [string] $GitCommitParameters
)

# To ensure a clean synchronization remove all files at the destination.
# This ensures that files no longer present in the build output do not
# persist in later versions.
Remove-Item "$VcpkgBetaFolder/ports/$VcpkgPortName" -Recurse -Force
New-Item -ItemType Directory -Path "$VcpkgBetaFolder/ports/$VcpkgPortName"

Copy-Item `
    -Path "$ReleaseArtifactSourceDirectory/vcpkg/port/*" `
    -Destination "$VcpkgBetaFolder/ports/$VcpkgPortName"

$rawPackageInfo = Get-Content -Raw -Path $ReleaseArtifactSourceDirectory/package-info.json
$packageInfo = ConvertFrom-Json $rawPackageInfo

# TODO: test?
# vcpkg install azure-core-cpp --overlay-ports=../azure-sdk-vcpkg-betas/ports/

$originalLocation = Get-Location
try {
    Set-Location $VcpkgBetaFolder

    Write-Host "$VcpkgFolder/vcpkg format-manifest --all --vcpkg-root=. --x-scripts-root=$VcpkgFolder/scripts"
    & $VcpkgFolder/vcpkg format-manifest `
        --all `
        --vcpkg-root=. `
        --x-scripts-root=$VcpkgFolder/scripts

    Write-Host "git add -A"
    git add -A
    Write-Host "git $GitCommitParameters commit -m \"$(Get-Date -Format "yyyy-MM-dd" ): $VcpkgPortName $($packageInfo.version)\""
    git $GitCommitParameters commit -m "$(Get-Date -Format "yyyy-MM-dd" ): $VcpkgPortName $($packageInfo.version)"

    Write-Host "$VcpkgFolder/vcpkg x-add-version --all --vcpkg-root=. --x-scripts-root=$VcpkgFolder/scripts"
    & $VcpkgFolder/vcpkg x-add-version `
        --all `
        --vcpkg-root=. `
        --x-scripts-root=$VcpkgFolder/scripts

    Write-Host "git add -A"
    git add -A
    Write-Host "git commit --amend --no-edit"
    git commit --amend --no-edit

    Write-Host "git log -1 --format=format:%H"
    $baseHash = git log -1 --format=format:%H
    Write-Host "New Baseline: $baseHash"

    # Update vcpkg-configuration.json to include this package and set the
    # baseline
    $vcpkgConfigPath = "$VcpkgBetaFolder/vcpkg-configuration.json"
    $rawVcpkgConfig = Get-Content -Raw -Path $vcpkgConfigPath
    $vcpkgConfig = ConvertFrom-Json $rawVcpkgConfig


    $vcpkgConfig.registries[0].baseline = $baseHash
    if (!($vcpkgConfig.registries[0].packages -contains $VcpkgPortName)) {
        $vcpkgConfig.registries[0].packages += $VcpkgPortName
    }

    $vcpkgConfigJson = ConvertTo-Json $vcpkgConfig -Depth 100
    Set-Content -Path $vcpkgConfigPath -Value $vcpkgConfigJson

} finally {
    Set-Location $originalLocation
}
