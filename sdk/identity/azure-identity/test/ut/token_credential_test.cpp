// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/identity/environment_credential.hpp>

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

namespace Azure { namespace Identity { namespace Test {
  class TokenCredentialTest : public Azure::Core::Test::TestBase {

  protected:
    // Required to rename the test properly once the test is started.
    // We can only know the test instance name until the test instance is run.
    std::unique_ptr<Azure::Identity::ClientSecretCredential> GetClientSecretCredential(
        std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);

      Azure::Core::Credentials::TokenCredentialOptions options = GetTokenCredentialOptions();
      return std::make_unique<Azure::Identity::ClientSecretCredential>(
          GetEnv("AZURE_TENANT_ID"),
          GetEnv("AZURE_CLIENT_ID"),
          GetEnv("AZURE_CLIENT_SECRET"),
          options);
    }

    // Required to rename the test properly once the test is started.
    // We can only know the test instance name until the test instance is run.
    std::unique_ptr<Azure::Identity::EnvironmentCredential> GetEnvironmentCredential(
        std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);

      Azure::Core::Credentials::TokenCredentialOptions options = GetTokenCredentialOptions();
      return std::make_unique<Azure::Identity::EnvironmentCredential>(options);
    }

    // Runs before every test.
    virtual void SetUp() override
    {
      Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
    }
  };
}}} // namespace Azure::Identity::Test

using namespace Azure::Identity::Test;
using namespace Azure::Identity;

TEST_F(TokenCredentialTest, ClientSecret)
{
  if (m_testContext.IsLiveMode())
  {
    GTEST_SKIP_(
        "Skipping ClientSecret test since it requires env vars that aren't set in live mode.");
  }

  std::string const testName(GetTestName());
  auto const clientSecretCredential = GetClientSecretCredential(testName);

  Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://vault.azure.net/.default"};
  tokenRequestContext.MinimumExpiration = std::chrono::hours(1000000);

  auto const token = clientSecretCredential->GetToken(
      tokenRequestContext, Azure::Core::Context::ApplicationContext);

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}

TEST_F(TokenCredentialTest, EnvironmentCredential)
{
  if (m_testContext.IsLiveMode())
  {
    GTEST_SKIP_("Skipping EnvironmentCredential test since it requires env vars that aren't set in "
                "live mode.");
  }

  std::string const testName(GetTestName());
  auto const clientSecretCredential = GetEnvironmentCredential(testName);

  Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://vault.azure.net/.default"};
  tokenRequestContext.MinimumExpiration = std::chrono::hours(1000000);

  auto const token = clientSecretCredential->GetToken(
      tokenRequestContext, Azure::Core::Context::ApplicationContext);

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}
