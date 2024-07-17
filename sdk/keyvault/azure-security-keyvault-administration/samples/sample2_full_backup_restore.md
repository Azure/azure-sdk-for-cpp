# Getting, updating, settings

This sample demonstrates how to perform full backup and full restore for an Azure Key Vault HSM.
To get started, you'll need a URI to an Azure Key Vault HSM.

## Creating a BackupRestoreClient

To create a new `BackupRestoreClient` to perform these operations, you need the endpoint to an Azure Key Vault HSM and credentials.

Key Vault BackupRestoreClient client for C++ currently supports any `TokenCredential` for authenticating.

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
 Azure::Core::Url blobUrl = Azure::Core::Url(
    Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_URL"));
```

## The Backup operation 

Since this is a long running operation the service provides endpoints to determine the status while the opperation is running. 

### Starting the backup operation

```cpp Snippet:SampleAdministration2StartBackup
// Create a full backup using a user-provided SAS token to an Azure blob storage container.
auto backupResponse = client.FullBackup(blobUrl, sasTokenParameter).Value;

std::cout << "Backup Job Id: " << backupResponse.Value().JobId << std::endl
            << "Backup Status: " << backupResponse.Value().Status << std::endl;
```

### Backup operation waiting 

In order to wait for the operation to complete we will call the polling method.

```cpp Snippet:SampleAdministration2StatusBackup
// Wait for the operation to complete.
auto backupStatus = backupResponse.PollUntilDone(10s);

std::cout << "Backup Job Id: " << backupStatus.Value.JobId << std::endl
            << "Backup Status: " << backupStatus.Value.Status << std::endl;
```

## The FullRestore operation

Similar to the backup operation after we initialize the operation we can check the status. 

### Starting the restore operation 

the restore operation requires a folder where a backup was previously performed along side the SAS token parameter. 
```cpp Snippet:SampleAdministration2FullRestoreStart
// Restore the full backup using a user-provided SAS token to an Azure blob storage container.
std::cout << "Folder to restore: " << folderToRestore << std::endl;
auto restoreResponse = client.FullRestore(blobUrl, folderToRestore, sasTokenParameter).Value;
std::cout << "Restore Job Id: " << restoreResponse.Value().JobId << std::endl
            << "Restore Status: " << restoreResponse.Value().Status << std::endl;
```

### FullRestore operation waiting

```cpp Snippet:SampleAdministration2StatusRestore
// Wait for the operation to complete.
auto restoreStatus = restoreResponse.PollUntilDone(10s);
std::cout << "Restore Job Id: " << restoreStatus.Value.JobId << std::endl
            << "Restore Status: " << restoreStatus.Value.Status << std::endl;
```
## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-administration/samples/sample2-full-backup-restore)
