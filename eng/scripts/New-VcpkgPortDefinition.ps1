<#
.SYNOPSIS

.DESCRIPTION

.PARAMETER SourceDirectory

.PARAMETER PackageSpecPath
Location of the package.json file.

.PARAMETER Destination

.PARAMETER Workspace
Workspace folder where assets are staged before creating.

#>


param (
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $SourceDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $PackageSpecPath,

    [string] $Url,

    [string] $Filename,

    [string] $Sha512
)

$initialLocation = Get-Location

try {
    $packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json

    $files = Get-ChildItem -Path $SourceDirectory

    foreach ($file in $files) {
        $content = Get-Content -Raw -Path $file
        $newContent = $content `
            -replace '%VERSION%', $packageSpec.version `
            -replace '%URL%', $Url `
            -replace '%FILENAME%', $Filename `
            -replace '%SHA512%', $Sha512

        $newContent | Set-Content $file
    }
} finally {
    Set-Location $initialLocation
}
