// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/administration/backup_restore_client.hpp"
#include "backup_restore_client_base_test.hpp"

#include <azure/core/base64.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/administration/rest_client_models.hpp>

#include <cstddef>
#include <string>
#include <thread>

#include <gtest/gtest.h>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault::Administration::Models;
using namespace Azure::Security::KeyVault::Administration::Test;

using namespace std::chrono_literals;

TEST_F(BackupRestoreClientTest, BackupFull)
{
  auto testName = "FullBackup";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();

  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
}

TEST_F(BackupRestoreClientTest, BackupFullStatus)
{
  auto testName = "FullBackup";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();

  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
  auto response2 = client.FullBackupStatus(response.Value.JobId);
  while (response2.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response2 = client.FullBackupStatus(response.Value.JobId);
  }
  EXPECT_EQ(response2.Value.Status, "Succeeded");
  EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
  EXPECT_FALSE(response2.Value.Error.HasValue());
  EXPECT_EQ(response.Value.JobId, response2.Value.JobId);
}

TEST_F(BackupRestoreClientTest, BackupFullErrorStatus)
{
  auto testName = "FullBackup";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();
  Azure::Core::Url url(sasTokenParameter.StorageResourceUri);
  sasTokenParameter.StorageResourceUri = url.GetScheme() + "://" + url.GetHost(); // invalid uri
  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
  auto response2 = client.FullBackupStatus(response.Value.JobId);
  while (response2.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response2 = client.FullBackupStatus(response.Value.JobId);
  }
  EXPECT_EQ(response2.Value.Status, "Failed");
  EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
  EXPECT_EQ(response2.Value.StatusDetails.Value(), "InvalidQueryParameterValue");
  EXPECT_EQ(response.Value.JobId, response2.Value.JobId);
}

TEST_F(BackupRestoreClientTest, RestoreFull)
{
  auto testName = "RestoreFull";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();

  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
  auto response2 = client.FullBackupStatus(response.Value.JobId);
  while (response2.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response2 = client.FullBackupStatus(response.Value.JobId);
  }
  EXPECT_EQ(response2.Value.Status, "Succeeded");
  EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
  EXPECT_FALSE(response2.Value.Error.HasValue());
  EXPECT_EQ(response.Value.JobId, response2.Value.JobId);

  RestoreOperationParameters restoreBlobDetails;
  restoreBlobDetails.SasTokenParameters = sasTokenParameter;
  Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
  auto subPath = url.GetPath();

  restoreBlobDetails.FolderToRestore = subPath.substr(7, subPath.size() - 1);

  auto response3 = client.FullRestore(restoreBlobDetails);
  EXPECT_EQ(response3.Value.Status, "InProgress");
  EXPECT_TRUE(response3.Value.StartTime > response3.Value.StartTime.min());
  EXPECT_FALSE(response3.Value.EndTime.HasValue());
}

TEST_F(BackupRestoreClientTest, RestoreFullStatus)
{
  auto testName = "RestoreFull";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();

  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
  auto response2 = client.FullBackupStatus(response.Value.JobId);
  while (response2.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response2 = client.FullBackupStatus(response.Value.JobId);
  }
  EXPECT_EQ(response2.Value.Status, "Succeeded");
  EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
  EXPECT_FALSE(response2.Value.Error.HasValue());
  EXPECT_EQ(response.Value.JobId, response2.Value.JobId);

  RestoreOperationParameters restoreBlobDetails;
  restoreBlobDetails.SasTokenParameters = sasTokenParameter;
  Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
  auto subPath = url.GetPath();

  restoreBlobDetails.FolderToRestore = subPath.substr(7, subPath.size() - 1);

  auto response3 = client.FullRestore(restoreBlobDetails);
  EXPECT_EQ(response3.Value.Status, "InProgress");
  EXPECT_TRUE(response3.Value.StartTime > response3.Value.StartTime.min());
  EXPECT_FALSE(response3.Value.EndTime.HasValue());
  auto response4 = client.RestoreStatus(response3.Value.JobId);
  while (response4.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response4 = client.RestoreStatus(response3.Value.JobId);
  }
  EXPECT_EQ(response4.Value.Status, "Succeeded");
  EXPECT_TRUE(response4.Value.EndTime.Value() > response4.Value.StartTime);
  EXPECT_FALSE(response4.Value.Error.HasValue());
  EXPECT_EQ(response3.Value.JobId, response4.Value.JobId);
}

TEST_F(BackupRestoreClientTest, RestoreSelectiveStatus)
{
  auto testName = "RestoreFull";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();

  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
  auto response2 = client.FullBackupStatus(response.Value.JobId);
  while (response2.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response2 = client.FullBackupStatus(response.Value.JobId);
  }
  EXPECT_EQ(response2.Value.Status, "Succeeded");
  EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
  EXPECT_FALSE(response2.Value.Error.HasValue());
  EXPECT_EQ(response.Value.JobId, response2.Value.JobId);

  SelectiveKeyRestoreOperationParameters restoreBlobDetails;
  restoreBlobDetails.SasTokenParameters = sasTokenParameter;

  Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
  auto subPath = url.GetPath();
  restoreBlobDetails.Folder = subPath.substr(7, subPath.size() - 1);

  auto response3 = client.SelectiveKeyRestore("trytry", restoreBlobDetails);
  EXPECT_EQ(response3.Value.Status, "InProgress");
  EXPECT_TRUE(response3.Value.StartTime > response3.Value.StartTime.min());
  EXPECT_FALSE(response3.Value.EndTime.HasValue());
  auto response4 = client.RestoreStatus(response3.Value.JobId);
  while (response4.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response4 = client.RestoreStatus(response3.Value.JobId);
  }
  EXPECT_EQ(response4.Value.Status, "Succeeded");
  EXPECT_TRUE(response4.Value.EndTime.Value() > response4.Value.StartTime);
  EXPECT_FALSE(response4.Value.Error.HasValue());
  EXPECT_EQ(response3.Value.JobId, response4.Value.JobId);
}

TEST_F(BackupRestoreClientTest, RestoreSelectiveInvalidKeyStatus)
{
  auto testName = "RestoreFull";
  CreateHSMClientForTest();
  auto& client = GetClientForTest(testName);
  SasTokenParameter sasTokenParameter = GetSasTokenBackup();

  auto response = client.FullBackup(sasTokenParameter);

  EXPECT_EQ(response.Value.Status, "InProgress");
  EXPECT_TRUE(response.Value.StartTime > response.Value.StartTime.min());
  EXPECT_FALSE(response.Value.EndTime.HasValue());
  EXPECT_FALSE(response.Value.Error.HasValue());
  auto response2 = client.FullBackupStatus(response.Value.JobId);
  while (response2.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response2 = client.FullBackupStatus(response.Value.JobId);
  }
  EXPECT_EQ(response2.Value.Status, "Succeeded");
  EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
  EXPECT_FALSE(response2.Value.Error.HasValue());
  EXPECT_EQ(response.Value.JobId, response2.Value.JobId);

  SelectiveKeyRestoreOperationParameters restoreBlobDetails;
  restoreBlobDetails.SasTokenParameters = sasTokenParameter;

  Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
  auto subPath = url.GetPath();
  restoreBlobDetails.Folder = subPath.substr(7, subPath.size() - 1);

  auto response3 = client.SelectiveKeyRestore("trytry2", restoreBlobDetails);
  EXPECT_EQ(response3.Value.Status, "InProgress");
  EXPECT_TRUE(response3.Value.StartTime > response3.Value.StartTime.min());
  EXPECT_FALSE(response3.Value.EndTime.HasValue());
  auto response4 = client.RestoreStatus(response3.Value.JobId);
  while (response4.Value.Status == "InProgress")
  {
    std::this_thread::sleep_for(5s);
    response4 = client.RestoreStatus(response3.Value.JobId);
  }
  EXPECT_EQ(response4.Value.Status, "Failed");
  EXPECT_TRUE(response4.Value.EndTime.Value() > response4.Value.StartTime);
  EXPECT_EQ(response4.Value.StatusDetails.Value(), "The given key or its versions NOT found");
  EXPECT_EQ(response3.Value.JobId, response4.Value.JobId);
}
