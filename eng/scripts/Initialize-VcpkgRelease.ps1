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

    [string] $GitRepo
)

# Upload the asset to the release
$assetInfo = & $PSScriptRoot/New-ReleaseAsset.ps1 `
    -SourceDirectory $SourceDirectory `
    -PackageSpecPath $PackageSpecPath `
    -GitRepo $GitRepo `
    -Username $Username `
    -Pass $Pass


$packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
$assetFilename = "$($packageSpec.assetName).tar.gz"

# Use asset URL to fill in vcpkg port tokens
& $PSScriptRoot/New-VcpkgPortDefinition.ps1 `
    -SourceDirectory "$SourceDirectory/port" `
    -PackageSpecPath $PackageSpecPath `
    -Url $assetInfo.browser_download_url `
    -Filename $assetFilename `
    -Sha512 (vcpkg.exe hash "$SourceDirectory/$assetFilename")
