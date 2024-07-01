// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Settings SDK client for
 * C++ to get one or more settings, and update a setting value.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_HSM_URL:  To the Key Vault HSM URL.
 * - AZURE_KEYVAULT_BACKUP_TOKEN : The SAS token to access the blob storage account for
 * backup/restore
 * - AZURE_KEYVAULT_BACKUP_URL : The URL to the blob storage account
 *
 */

#include <azure/identity.hpp>
#include <azure/keyvault/administration.hpp>

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Administration;
using namespace std::chrono_literals;

int main()
{
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  // create client
  BackupRestoreClient client(
      Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_HSM_URL"), credential);
  SasTokenParameter sasTokenParameter;
  // the backup/restore needs a SAS token to access the storage account
  sasTokenParameter.Token
      = Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_TOKEN");
  // the backup/restore needs a url to a blob storage resource
  Azure::Core::Url blobUrl = Azure::Core::Url(
      Azure::Core::_internal::Environment::GetVariable("AZURE_KEYVAULT_BACKUP_URL"));

  try
  {
    // Create a full backup using a user-provided SAS token to an Azure blob storage container.
    auto backupResponse = client.FullBackup(blobUrl, sasTokenParameter);

    std::cout << "Backup Job Id: " << backupResponse.Value.JobId << std::endl
              << "Backup Status: " << backupResponse.Value.Status << std::endl;
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

    // Restore the full backup using a user-provided SAS token to an Azure blob storage container.
    Azure::Core::Url url(backupStatus.Value.AzureStorageBlobContainerUri);
    auto subPath = url.GetPath();
    std::string folderToRestore = subPath.substr(7, subPath.size() - 1);

    std::cout << "Folder to restore: " << folderToRestore << std::endl;
    auto restoreResponse = client.FullRestore(blobUrl, folderToRestore, sasTokenParameter);
    std::cout << "Restore Job Id: " << restoreResponse.Value.JobId << std::endl
              << "Restore Status: " << restoreResponse.Value.Status << std::endl;

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
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Key Vault Settings Client Exception happened:" << std::endl
              << e.Message << std::endl;
    return 1;
  }

  return 0;
}
