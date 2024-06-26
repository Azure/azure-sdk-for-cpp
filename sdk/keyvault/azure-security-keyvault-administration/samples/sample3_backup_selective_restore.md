# Getting, updating, settings

This sample demonstrates how to perform full backup and full restore for an Azure Key Vault HSM.
To get started, you'll need a URI to an Azure Key Vault HSM.

## Creating a BackupRestoreClient

To create a new `BackupRestoreClient` to perform these operations, you need the endpoint to an Azure Key Vault HSM and credentials.

Key Vault BackupRerstoreClient client for C++ currently supports any `TokenCredential` for authenticating.

```cpp Snippet:SampleBackupRestoreCreateCredential
  auto credential
      = std::make_shared<Azure::Identity::DefaultAzureCredential>();
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SampleAdministration2CreateClient
  // create client
  BackupRestoreClient client(std::getenv("AZURE_KEYVAULT_HSM_URL"), credential);
```
## Create the SasTokenParameter

Since these operations require a blob storage for the backup/restore operations, a SAS token is required for the connection between the services(Key Vault and Storage).  

In this sample we rely on a couple of extra environment variables. 

```cpp Snippet:SampleAdministration2CreateSASParam
 SasTokenParameter sasTokenParameter;
 // the backup/restore needs a SAS token to access the storage account
 sasTokenParameter.Token
     = Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_TOKEN");
 // the backup/restore needs a url to a blob storage resource
 sasTokenParameter.StorageResourceUri
     = Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_URL");
```

## The Backup operation 

Since this is a long running operation the service provides endpoints to determine the status while the opperation is running. 

### Starting the backup operation

```cpp Snippet:SampleAdministration2StartBackup
// Create a full backup using a user-provided SAS token to an Azure blob storage container.
auto backupResponse = client.FullBackup(sasTokenParameter);

std::cout << "Backup Job Id: " << backupResponse.Value.JobId << std::endl
          << "Backup Status: " << backupResponse.Value.Status << std::endl;
```

### Backup status check 

In order to determine the status of the operation we need to call the corresponding endpoint.

```cpp Snippet:SampleAdministration2StatusBackup
// Get the status of the backup operation.
auto backupStatus = client.FullBackupStatus(backupResponse.Value.JobId);

// Wait for the backup to complete.
while (backupStatus.Value.Status == "InProgress")
{
  std::this_thread::sleep_for(5s);
  backupStatus = client.FullBackupStatus(backupStatus.Value.JobId);
}
std::cout << "Backup Job Id: " << backupStatus.Value.JobId << std::endl
          << "Backup Status: " << backupStatus.Value.Status << std::endl;
```

## The SelectiveRestore operation

Similar to the backup operation after we initialize the operation we can check the status. 

### Starting the restore operation 

The selective restore operation requires a folder where a backup was previously performed along side the SAS token parameter. 

```cpp Snippet:SampleAdministration2FullRestoreStart
// Restore the full backup using a user-provided SAS token to an Azure blob storage container.
RestoreOperationParameters restoreBlobDetails;
restoreBlobDetails.SasTokenParameters = sasTokenParameter;
restoreBlobDetails.FolderToRestore = "folder name here";
std::cout << "Folder to restore: " << restoreBlobDetails.FolderToRestore << std::endl;
auto selectiveRestore = client.SelectiveKeyRestore(keyName, restoreBlobDetails);
std::cout << "Restore Job Id: " << restoreResponse.Value.JobId << std::endl
          << "Restore Status: " << restoreResponse.Value.Status << std::endl;
```

### FullRestore status check

```cpp Snippet:SampleAdministration2StatusRestore
// Get the status of the restore operation.
auto restoreStatus = client.RestoreStatus(restoreResponse.Value.JobId);
// Wait for the restore to complete.
while (restoreStatus.Value.Status == "InProgress")
{
  std::this_thread::sleep_for(5s);
  restoreStatus = client.RestoreStatus(restoreStatus.Value.JobId);
}
std::cout << "Restore Job Id: " << restoreStatus.Value.JobId << std::endl
          << "Restore Status: " << restoreStatus.Value.Status << std::endl;
```
## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-administration/samples/sample3-backup-selective-restore)
