<#
.SYNOPSIS
Replaces token content in each file of the SourceDirectory (assumes no subfolders)

.PARAMETER SourceDirectory
Location of vcpkg port files
(usually `<artifact-path>/packages/<package-name>/vcpkg/port`)

.PARAMETER Version
Replaces %VERSION% token in files with the value of this parameter

.PARAMETER Url
Replaces %URL% token in files with the value of this parameter

.PARAMETER Filename
Replaces %FILENAME% token in files with the value of this parameter

.PARAMETER Sha512
Replaces %SHA512% token in files with the value of this parameter

#>


param (
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $SourceDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Version,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Url,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Filename,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Sha512
)

$files = Get-ChildItem -Path $SourceDirectory

foreach ($file in $files) {
    $content = Get-Content -Raw -Path $file
    $newContent = $content `
        -replace '%VERSION%', $Version `
        -replace '%URL%', $Url `
        -replace '%FILENAME%', $Filename `
        -replace '%SHA512%', $Sha512

    $newContent | Set-Content $file
}
