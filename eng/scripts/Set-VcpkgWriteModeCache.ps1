param(
    [string] $StorageAccountKey
)


."$PSScriptRoot/../common/scripts/Helpers/PSModule-Helpers.ps1"

Write-Host "`$env:PSModulePath = $($env:PSModulePath)"

# Work around double backslash
if ($IsWindows) {
    $hostedAgentModulePath = $env:SystemDrive + "\\Modules"
    $moduleSeperator = ";"
} else {
    $hostedAgentModulePath = "/usr/share"
    $moduleSeperator = ":"
}
$modulePaths = $env:PSModulePath -split $moduleSeperator
$modulePaths = $modulePaths.Where({ !$_.StartsWith($hostedAgentModulePath) })
$AzModuleCachPath = (Get-ChildItem "$hostedAgentModulePath/az_*" -Attributes Directory) -join $moduleSeperator
if ($AzModuleCachPath -and $env.PSModulePath -notcontains $AzModuleCachPath) {
    $modulePaths += $AzModuleCachPath
}

$env:PSModulePath = $modulePaths -join $moduleSeperator

try {
    Write-Host "Get-Command Start-CopyAzureStorageBlob | Format-List"
    Get-Command "Start-CopyAzureStorageBlob" | Format-List
    # It's an alias to Start-AzureStorageBlobCopy
    Write-Host "Get-Command Start-AzureStorageBlobCopy | Format-List"
    Get-Command "Start-AzureStorageBlobCopy" | Format-List

} catch {
    Write-Host "Start-CopyAzureStorageBlob does not exist"
}


Install-ModuleIfNotInstalled "Az.Storage" "4.3.0" | Import-Module

$ctx = New-AzStorageContext `
    -StorageAccountName 'cppvcpkgcache' `
    -StorageAccountKey $StorageAccountKey
$token = New-AzStorageAccountSASToken `
    -Service Blob `
    -ResourceType Object `
    -Permission "rwc" `
    -Context $ctx `
    -ExpiryTime (Get-Date).AddDays(1)
$vcpkgBinarySourceSas = $token.Substring(1)

Write-Host "Setting vcpkg binary cache to read and write"
Write-Host "##vso[task.setvariable variable=VCPKG_BINARY_SOURCES_SECRET;issecret=true;]clear;x-azblob,https://cppvcpkgcache.blob.core.windows.net/public-vcpkg-container,$vcpkgBinarySourceSas,readwrite"
