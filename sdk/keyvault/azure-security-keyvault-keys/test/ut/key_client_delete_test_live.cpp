// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/keys.hpp>
#include <private/key_constants.hpp>

#include <string>

namespace {

std::string GetConflictErrorMsg(std::string const& keyName)
{
  return "Key " + keyName
      + " is currently in a deleted but recoverable state, and its name cannot be reused; in this "
        "state, the key can only be recovered or purged.";
}

std::string GetConflictDeletingErrorSlow(std::string const& keyName)
{
  return "Key " + keyName + " is currently being deleted and cannot be re-created; retry later.";
}
} // namespace

using namespace Azure::Security::KeyVault::Keys::Test;

// Test Key Delete.
// The test works for either soft-delete or not, but for non soft-delete, the LRO is completed as
// soon as the Operation returns.
TEST_F(KeyVaultKeyClient, DeleteKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Setting a timeout context to avoid this test to run up to ctest default timeout
    // The polling operation would usually complete in ~20 seconds.
    // Setting 3 min as timeout just because I like number 3. We just want to prevent test running
    // for so long if something happens and no exception is thrown (paranoid scenario)
    auto duration
        = std::chrono::system_clock::now() + std::chrono::minutes(m_testPollingTimeOutMinutes);
    auto cancelToken = Azure::Core::Context::ApplicationContext.WithDeadline(duration);

    auto keyResponseLRO = client.StartDeleteKey(keyName);
    auto expectedStatusToken = keyName;
    EXPECT_EQ(keyResponseLRO.GetResumeToken(), expectedStatusToken);
    // poll each second until key is soft-deleted
    // Will throw and fail test if test takes more than 3 minutes (token cancelled)
    // double polling should not interfere with the outcome
    auto keyResponse = keyResponseLRO.PollUntilDone(m_testPollingIntervalMs, cancelToken);
    keyResponse = keyResponseLRO.PollUntilDone(m_testPollingIntervalMs, cancelToken);
  }
}

TEST_F(KeyVaultKeyClient, DeleteKeyOperationPoll)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
    auto pollResponse = keyResponseLRO.Poll();
    // Expected not completed operation
    EXPECT_EQ(
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            pollResponse.GetStatusCode()),
        404);
  }
}

// Delete key which doesn't exists
TEST_F(KeyVaultKeyClient, DeleteInvalidKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  auto wasThrown = false;
  try
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
  }
  catch (Azure::Core::RequestFailedException const& error)
  {
    EXPECT_EQ(
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            error.StatusCode),
        404);
    EXPECT_EQ(error.ErrorCode, "KeyNotFound");
    wasThrown = true;
  }
  catch (std::exception const&)
  {
    throw;
  }
  EXPECT_TRUE(wasThrown);
}

TEST_F(KeyVaultKeyClient, DoubleDelete)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
  }
  {
    auto duration
        = std::chrono::system_clock::now() + std::chrono::minutes(m_testPollingTimeOutMinutes);
    auto cancelToken = Azure::Core::Context::ApplicationContext.WithDeadline(duration);
    auto keyResponseLRO = client.StartDeleteKey(keyName);
    auto keyResponse = keyResponseLRO.PollUntilDone(m_testPollingIntervalMs, cancelToken);
  }
  // delete same key again
  auto wasThrown = false;
  try
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
  }
  catch (Azure::Core::RequestFailedException const& error)
  {
    EXPECT_EQ(
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            error.StatusCode),
        404);
    EXPECT_EQ(error.ErrorCode, "KeyNotFound");
    wasThrown = true;
  }
  catch (std::exception const&)
  {
    throw;
  }
  EXPECT_TRUE(wasThrown);
}

TEST_F(KeyVaultKeyClient, DoubleDeleteBeforePollComplete)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
  }
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
  }
  // delete same key again before waitting for poll complete
  auto wasThrown = false;
  try
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
  }
  catch (Azure::Core::RequestFailedException const& error)
  {
    EXPECT_EQ(
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            error.StatusCode),
        404);

    EXPECT_EQ(error.ErrorCode, "KeyNotFound");
    wasThrown = true;
  }
  catch (std::exception const&)
  {
    throw;
  }
  EXPECT_TRUE(wasThrown);
}

TEST_F(KeyVaultKeyClient, CreateDeletedKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
  }
  {
    auto duration
        = std::chrono::system_clock::now() + std::chrono::minutes(m_testPollingTimeOutMinutes);
    auto cancelToken = Azure::Core::Context::ApplicationContext.WithDeadline(duration);
    auto keyResponseLRO = client.StartDeleteKey(keyName);
    auto keyResponse = keyResponseLRO.PollUntilDone(m_testPollingIntervalMs, cancelToken);
  }
  // Create a key with same name
  auto wasThrown = false;
  try
  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
  }
  catch (Azure::Core::RequestFailedException const& error)
  {
    EXPECT_EQ(
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            error.StatusCode),
        409);
    EXPECT_EQ(error.Message, GetConflictErrorMsg(keyName));
    EXPECT_EQ(error.ErrorCode, "Conflict");

    wasThrown = true;
  }
  catch (std::exception const&)
  {
    throw;
  }
  EXPECT_TRUE(wasThrown);
}

TEST_F(KeyVaultKeyClient, CreateDeletedKeyBeforePollComplete)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
  }
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
  }
  // Create a key with same name
  auto wasThrown = false;
  try
  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
  }
  catch (Azure::Core::RequestFailedException const& error)
  {
    EXPECT_EQ(
        static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
            error.StatusCode),
        409);
    EXPECT_EQ(error.ErrorCode, "Conflict");
    EXPECT_TRUE(
        error.Message.compare(GetConflictDeletingErrorSlow(keyName)) == 0
        || error.Message.compare(GetConflictErrorMsg(keyName)) == 0);
    wasThrown = true;
  }
  catch (std::exception const&)
  {
    throw;
  }
  EXPECT_TRUE(wasThrown);
}

// Get Delete Key
TEST_F(KeyVaultKeyClient, GetDeletedKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Wait until key is deleted
    auto duration
        = std::chrono::system_clock::now() + std::chrono::minutes(m_testPollingTimeOutMinutes);
    auto cancelToken = Azure::Core::Context::ApplicationContext.WithDeadline(duration);

    auto keyResponseLRO = client.StartDeleteKey(keyName);
    auto expectedStatusToken = m_keyVaultUrl
        + std::string(Azure::Security::KeyVault::Keys::_detail::DeletedKeysPath) + "/" + keyName;
    auto keyResponse = keyResponseLRO.PollUntilDone(m_testPollingIntervalMs, cancelToken);
  }
  {
    // Get the deleted key
    auto deletedKey = client.GetDeletedKey(keyName).Value;
    EXPECT_FALSE(deletedKey.RecoveryId.empty());
    EXPECT_EQ(deletedKey.Name(), keyName);
    auto expectedType = Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec;
    EXPECT_EQ(expectedType, deletedKey.Key.KeyType);
  }
}

TEST_F(KeyVaultKeyClient, DeleteOperationResumeToken)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  std::string resumeToken;
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
    resumeToken = keyResponseLRO.GetResumeToken();
  }
  // Resume operation from token
  {
    auto resumeOperation
        = Azure::Security::KeyVault::Keys::DeleteKeyOperation::CreateFromResumeToken(
            resumeToken, client);
    resumeOperation.PollUntilDone(m_testPollingIntervalMs);
  }
}

TEST_F(KeyVaultKeyClient, RecoverOperationResumeToken)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  std::string resumeToken;
  {
    auto keyResponseLRO = client.StartDeleteKey(keyName);
    resumeToken = keyResponseLRO.GetResumeToken();
  }
  // Resume operation from token
  {
    auto resumeOperation
        = Azure::Security::KeyVault::Keys::DeleteKeyOperation::CreateFromResumeToken(
            resumeToken, client);
    // double polling should have no impact on the result
    resumeOperation.PollUntilDone(m_testPollingIntervalMs);
    resumeOperation.PollUntilDone(m_testPollingIntervalMs);
  }
  {
    // recover
    auto recoverOperation = client.StartRecoverDeletedKey(keyName);
    resumeToken = recoverOperation.GetResumeToken();
  }
  {
    // resume from token
    auto resumeRecoveryOp
        = Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation::CreateFromResumeToken(
            resumeToken, client);
    // double polling should have no impact on the result
    auto keyResponse = resumeRecoveryOp.PollUntilDone(m_testPollingIntervalMs);
    keyResponse = resumeRecoveryOp.PollUntilDone(m_testPollingIntervalMs);
    auto key = keyResponse.Value;
  }
}
