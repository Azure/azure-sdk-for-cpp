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
New-Item -ItemType Directory -Path "$VcpkgBetaFolder/vcpkg/ports/$VcpkgPortName"

Copy-Item `
    -Path "$ReleaseArtifactSourceDirectory/vcpkg/port/*" `
    -Destination "$VcpkgBetaFolder/vcpkg/ports/$VcpkgPortName"

$rawPackageInfo = Get-Content -Raw -Path $ReleaseArtifactSourceDirectory/package-info.json
$packageInfo = ConvertFrom-Json $rawPackageInfo

$portName = "$($packageInfo.name)-cpp"

# TODO: test?
# vcpkg install azure-core-cpp --overlay-ports=../azure-sdk-vcpkg-betas/ports/

$originalLocation = Get-Location
try {
    & $VcpkgFolder/vcpkg format-manifest `
        --all `
        --vcpkg-root =. `
        --x-scripts-root=$VcpkgFolder/scripts

    Write-Host "git add -A"
    git add -A
    Write-Host "git $GitCommitParameters commit -m \"$(Get-Date -Format "yyyy-MM-dd" ): $portName $($packageInfo.version)\""
    git $GitCommitParameters commit -m "$(Get-Date -Format "yyyy-MM-dd" ): $portName $($packageInfo.version)"

    & $VcpkgFolder/vcpkg x-add-version `
        --all `
        --vcpkg-root=. `
        --x-scripts-root=$VcpkgFolder/scripts

    git add -A
    git commit --amend --no-edit

    $commitHash = git log -1 --format=format:%H

    # Update vcpkg-configuration.json to include this package and set the
    # baseline
    $vcpkgConfigPath = "$VcpkgBetaFolder/vcpkg-configuration.json"
    $rawVcpkgConfig = Get-Content -Raw -Path $vcpkgConfigPath
    $vcpkgConfig = ConvertFrom-Json $rawVcpkgConfig


    $vcpkgConfig.registries[0].baseline = $commitHash
    if (!($vcpkgConfig.registries[0].packages -contains $portName)) {
        $vcpkgConfig.registries[0].packages += $portName
    }

    $vcpkgConfigJson = ConvertTo-Json $vcpkgConfig -Depth 100
    Set-Content -Path $vcpkgConfigPath -Value $vcpkgConfigJson

} finally {
    Set-Location $originalLocation
}
