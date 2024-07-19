// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/administration/backup_client.hpp"
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

TEST_F(BackupRestoreClientTest, BackupFull_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "BackupFull";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();

    auto response = client.FullBackup(m_blobUrl, sasTokenParameter).Value;
    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());
  }
  else
  {
    SkipTest();
  }
}

TEST_F(BackupRestoreClientTest, BackupFullStatus_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "BackupFullStatus";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();

    auto response = client.FullBackup(m_blobUrl, sasTokenParameter).Value;

    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());
    auto response2 = response.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response2.Value.Status, "Succeeded");
    EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
    EXPECT_FALSE(response2.Value.Error.HasValue());
    EXPECT_EQ(response.Value().JobId, response2.Value.JobId);
  }
  else
  {
    SkipTest();
  }
}

TEST_F(BackupRestoreClientTest, BackupFullErrorStatus_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "BackupFullErrorStatus";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();
    Azure::Core::Url defectiveUrl(
        m_blobUrl.GetScheme() + "://" + m_blobUrl.GetHost()); // invalid uri
    auto response = client.FullBackup(defectiveUrl, sasTokenParameter).Value;

    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());

    response.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response.Value().Status, "Failed");
    EXPECT_TRUE(response.Value().EndTime.Value() > response.Value().StartTime);
    EXPECT_EQ(response.Value().StatusDetails.Value(), "InvalidQueryParameterValue");
    EXPECT_EQ(response.Value().Error.Value().Code, "InvalidQueryParameterValue");
  }
  else
  {
    SkipTest();
  }
}

TEST_F(BackupRestoreClientTest, RestoreFull_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "RestoreFull";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();

    auto response = client.FullBackup(m_blobUrl, sasTokenParameter).Value;

    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());
    auto response2 = response.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response2.Value.Status, "Succeeded");
    EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
    EXPECT_FALSE(response2.Value.Error.HasValue());
    EXPECT_EQ(response.Value().JobId, response2.Value.JobId);

    Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
    auto subPath = url.GetPath();
    std::string folderToRestore = subPath.substr(7, subPath.size() - 1);

    auto response3 = client.FullRestore(m_blobUrl, folderToRestore, sasTokenParameter).Value;
    EXPECT_EQ(response3.Value().Status, "InProgress");
    EXPECT_TRUE(response3.Value().StartTime > response3.Value().StartTime.min());
    EXPECT_FALSE(response3.Value().EndTime.HasValue());
  }
  else
  {
    SkipTest();
  }
}

TEST_F(BackupRestoreClientTest, RestoreFullStatus_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "RestoreFullStatus";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();

    auto response = client.FullBackup(m_blobUrl, sasTokenParameter).Value;

    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());
    auto response2 = response.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response2.Value.Status, "Succeeded");
    EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
    EXPECT_FALSE(response2.Value.Error.HasValue());
    EXPECT_EQ(response.Value().JobId, response2.Value.JobId);

    Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
    auto subPath = url.GetPath();
    std::string folderToRestore = subPath.substr(7, subPath.size() - 1);

    auto response3 = client.FullRestore(m_blobUrl, folderToRestore, sasTokenParameter).Value;
    EXPECT_EQ(response3.Value().Status, "InProgress");
    EXPECT_TRUE(response3.Value().StartTime > response3.Value().StartTime.min());
    EXPECT_FALSE(response3.Value().EndTime.HasValue());
    auto response4 = response3.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response4.Value.Status, "Succeeded");
    EXPECT_TRUE(response4.Value.EndTime.Value() > response4.Value.StartTime);
    EXPECT_FALSE(response4.Value.Error.HasValue());
    EXPECT_EQ(response3.Value().JobId, response4.Value.JobId);
  }
  else
  {
    SkipTest();
  }
}

TEST_F(BackupRestoreClientTest, RestoreSelectiveStatus_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "RestoreSelectiveStatus";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();

    auto response = client.FullBackup(m_blobUrl, sasTokenParameter).Value;

    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());
    auto response2 = response.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response2.Value.Status, "Succeeded");
    EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
    EXPECT_FALSE(response2.Value.Error.HasValue());
    EXPECT_EQ(response.Value().JobId, response2.Value.JobId);

    Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
    auto subPath = url.GetPath();
    std::string folderToRestore = subPath.substr(7, subPath.size() - 1);

    auto response3
        = client.SelectiveKeyRestore("trytry", m_blobUrl, folderToRestore, sasTokenParameter).Value;
    EXPECT_EQ(response3.Value().Status, "InProgress");
    EXPECT_TRUE(response3.Value().StartTime > response3.Value().StartTime.min());
    EXPECT_FALSE(response3.Value().EndTime.HasValue());
    auto response4 = response3.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response4.Value.Status, "Succeeded");
    EXPECT_TRUE(response4.Value.EndTime.Value() > response4.Value.StartTime);
    EXPECT_FALSE(response4.Value.Error.HasValue());
    EXPECT_EQ(response3.Value().JobId, response4.Value.JobId);
  }
  else
  {
    SkipTest();
  }
}

TEST_F(BackupRestoreClientTest, RestoreSelectiveInvalidKeyStatus_RECORDEDONLY_)
{
  if (m_keyVaultHsmUrl != m_keyVaultUrl)
  {
    auto testName = "RestoreSelectiveInvalidKeyStatus";
    CreateHSMClientForTest();
    auto& client = GetClientForTest(testName);
    SasTokenParameter sasTokenParameter = GetSasTokenBackup();

    auto response = client.FullBackup(m_blobUrl, sasTokenParameter).Value;

    EXPECT_EQ(response.Value().Status, "InProgress");
    EXPECT_TRUE(response.Value().StartTime > response.Value().StartTime.min());
    EXPECT_FALSE(response.Value().EndTime.HasValue());
    EXPECT_FALSE(response.Value().Error.HasValue());
    auto response2 = response.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response2.Value.Status, "Succeeded");
    EXPECT_TRUE(response2.Value.EndTime.Value() > response2.Value.StartTime);
    EXPECT_FALSE(response2.Value.Error.HasValue());
    EXPECT_EQ(response.Value().JobId, response2.Value.JobId);

    Azure::Core::Url url(response2.Value.AzureStorageBlobContainerUri);
    auto subPath = url.GetPath();
    std::string folderToRestore = subPath.substr(7, subPath.size() - 1);

    auto response3
        = client.SelectiveKeyRestore("trytry2", m_blobUrl, folderToRestore, sasTokenParameter)
              .Value;
    EXPECT_EQ(response3.Value().Status, "InProgress");
    EXPECT_TRUE(response3.Value().StartTime > response3.Value().StartTime.min());
    EXPECT_FALSE(response3.Value().EndTime.HasValue());
    auto response4 = response3.PollUntilDone(m_testPollingIntervalMs);
    EXPECT_EQ(response4.Value.Status, "Failed");
    EXPECT_TRUE(response4.Value.EndTime.Value() > response4.Value.StartTime);
    EXPECT_EQ(response4.Value.StatusDetails.Value(), "The given key or its versions NOT found");
    EXPECT_EQ(response3.Value().JobId, response4.Value.JobId);
    EXPECT_EQ(response4.Value.Error.Value().Message, "The given key or its versions NOT found");
    EXPECT_EQ(response4.Value.Error.Value().Code, "No key versions are updated");
  }
  else
  {
    SkipTest();
  }
}
