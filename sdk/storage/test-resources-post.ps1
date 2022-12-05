# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

param(
  [string] $ResourceGroupName,
  [hashtable] $DeploymentOutputs
)

New-AzStorageEncryptionScope -ResourceGroupName $ResourceGroupName -StorageAccountName $DeploymentOutputs['ACCOUNT_NAME'] -EncryptionScopeName "EncryptionScopeForTest" -StorageEncryption

New-AzStorageEncryptionScope -ResourceGroupName $ResourceGroupName -StorageAccountName $DeploymentOutputs['DATALAKE_ACCOUNT_NAME'] -EncryptionScopeName "EncryptionScopeForTest" -StorageEncryption

Enable-AzStorageBlobDeleteRetentionPolicy -ResourceGroupName $ResourceGroupName -StorageAccountName $DeploymentOutputs['DATALAKE_ACCOUNT_NAME'] -RetentionDays 7

# This script is used to wait until XCache is refreshed for the service properties (30s), and role assignment takes effect (300s).

Start-Sleep -s 300