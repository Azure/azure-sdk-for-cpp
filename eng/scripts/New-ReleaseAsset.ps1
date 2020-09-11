<#
.SYNOPSIS

.DESCRIPTION

.PARAMETER SourceDirectory

.PARAMETER PackageSpecPath
Location of the package.json file.

#>


param (
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $SourceDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $PackageSpecPath,

    [string] $GitRepo,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Username,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Pass
)

$packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
$assetFilename = "$($packageSpec.assetName).tar.gz"

# TODO: Change the URL
$release = Invoke-RestMethod `
    -Uri https://api.github.com/repos/$GitRepo/releases/tags/$($packageSpec.assetName) `
    -Method GET

$uploadUrl = $release.upload_url.Split('{')[0] + "?name=$assetFilename"

$securePass = ConvertTo-SecureString -String $Pass -AsPlainText -Force

$credentials = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $Username, $securePass

$asset = Invoke-RestMethod `
    -Uri $uploadUrl `
    -Method POST `
    -InFile $SourceDirectory/$assetFilename `
    -Credential $credentials `
    -Authentication Basic `
    -ContentType "application/gzip"

return $asset