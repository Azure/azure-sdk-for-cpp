#!/bin/env pwsh
param(
  [string] $StorageAccountName = 'azuresdkartifacts',
  [string] $StorageContainerName = 'public-vcpkg-container',
  [int] $TokenTimeoutInHours = 1
)

$ctx = New-AzStorageContext `
  -StorageAccountName $StorageAccountName `
  -UseConnectedAccount

$vcpkgBinarySourceSas = New-AzStorageContainerSASToken `
  -Name $StorageContainerName `
  -Permission "rwcl" `
  -Context $ctx `
  -ExpiryTime (Get-Date).AddHours($TokenTimeoutInHours)

# Ensure redaction of SAS tokens in logs
Write-Host "##vso[task.setvariable variable=VCPKG_BINARY_SAS_TOKEN;issecret=true;]$vcpkgBinarySourceSas"

Write-Host "Binary cache disabled (clear) to force full from-source builds and validate Terrapin asset coverage"
Write-Host "##vso[task.setvariable variable=VCPKG_BINARY_SOURCES_SECRET;issecret=true;]clear"
