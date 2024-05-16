# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

param(
  [string] $ResourceGroupName,
  [hashtable] $DeploymentOutputs
)

New-AzStorageEncryptionScope -ResourceGroupName $ResourceGroupName -StorageAccountName $DeploymentOutputs['ACCOUNT_NAME'] -EncryptionScopeName "EncryptionScopeForTest" -StorageEncryption

Enable-AzStorageBlobLastAccessTimeTracking -ResourceGroupName $ResourceGroupName -StorageAccountName $DeploymentOutputs['ACCOUNT_NAME'] -PassThru

# This script is used to wait until XCache is refreshed for the service properties (30s), and role assignment takes effect (300s).

Start-Sleep -s 300
