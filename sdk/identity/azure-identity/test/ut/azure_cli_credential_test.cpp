//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/azure_cli_credential.hpp"

#include <azure/core/platform.hpp>

#include <atomic>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>

using Azure::Identity::AzureCliCredential;

using Azure::DateTime;
using Azure::Core::Context;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Identity::AzureCliCredentialOptions;

namespace {
constexpr auto InfiniteCommand =
#if defined(AZ_PLATFORM_WINDOWS)
    "for /l %q in (0) do timeout 10";
#else
    "while true; do sleep 10; done"
#endif
;

constexpr auto EmptyOutputCommand =
#if defined(AZ_PLATFORM_WINDOWS)
    "rem";
#else
    "clear"
#endif
;

std::string EchoCommand(std::string const text)
{
#if defined(AZ_PLATFORM_WINDOWS)
  return std::string("echo ") + text;
#else
  return std::string("echo \'") + text + "\'";
#endif
}

class AzureCliTestCredential : public AzureCliCredential {
private:
  std::string m_command;

  std::string GetAzCommand(std::string const& resource, std::string const& tenantId) const override
  {
    static_cast<void>(resource);
    static_cast<void>(tenantId);

    return m_command;
  }

public:
  explicit AzureCliTestCredential(std::string command) : m_command(std::move(command)) {}

  explicit AzureCliTestCredential(std::string command, AzureCliCredentialOptions const& options)
      : AzureCliCredential(options), m_command(std::move(command))
  {
  }

  explicit AzureCliTestCredential(std::string command, TokenCredentialOptions const& options)
      : AzureCliCredential(options), m_command(std::move(command))
  {
  }

  std::string GetOriginalAzCommand(std::string const& resource, std::string const& tenantId) const
  {
    return AzureCliCredential::GetAzCommand(resource, tenantId);
  }

  decltype(m_tenantId) const& GetTenantId() const { return m_tenantId; }
  decltype(m_cliProcessTimeout) const& GetCliProcessTimeout() const { return m_cliProcessTimeout; }
};
} // namespace

TEST(AzureCliCredential, Success)
{
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\","
                         "\"expiresOn\":\"2022-08-24 00:43:08.000000\","
                         "\"tenant\":\"72f988bf-86f1-41af-91ab-2d7cd011db47\","
                         "\"tokenType\":\"Bearer\"}";

  AzureCliTestCredential const azCliCred(EchoCommand(Token));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");
  auto const token = azCliCred.GetToken(trc, {});

  EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  EXPECT_EQ(
      token.ExpiresOn,
      DateTime::Parse("2022-08-24T00:43:08.000000Z", DateTime::DateFormat::Rfc3339));
}

TEST(AzureCliCredential, Error)
{
  AzureCliTestCredential const azCliCred(
      EchoCommand("ERROR: Please run 'az login' to setup account."));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
}

TEST(AzureCliCredential, EmptyOutput)
{
  AzureCliTestCredential const azCliCred(EmptyOutputCommand);

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
}

TEST(AzureCliCredential, BigToken)
{
  std::string accessToken;
  {
    std::string const tokenPart = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    auto const nIterations = ((4 * 1024) / tokenPart.size()) + 1;
    for (auto i = 0; i < static_cast<decltype(i)>(nIterations); ++i)
    {
      accessToken += tokenPart;
    }
  }

  AzureCliTestCredential const azCliCred(EchoCommand(
      std::string("{\"accessToken\":\"") + accessToken
      + "\",\"expiresOn\":\"2022-08-24 00:43:08.000000\"}"));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  auto const token = azCliCred.GetToken(trc, {});

  EXPECT_EQ(token.Token, accessToken);

  EXPECT_EQ(
      token.ExpiresOn,
      DateTime::Parse("2022-08-24T00:43:08.000000Z", DateTime::DateFormat::Rfc3339));
}

TEST(AzureCliCredential, ExpiresIn)
{
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\",\"expiresIn\":30}";

  AzureCliTestCredential const azCliCred(EchoCommand(Token));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  auto const timestampBefore = std::chrono::system_clock::now();
  auto const token = azCliCred.GetToken(trc, {});
  auto const timestampAfter = std::chrono::system_clock::now();

  EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  EXPECT_GE(token.ExpiresOn, timestampBefore + std::chrono::seconds(30));
  EXPECT_LE(token.ExpiresOn, timestampAfter + std::chrono::seconds(30));
}

TEST(AzureCliCredential, TimedOut)
{
  AzureCliCredentialOptions options;
  options.CliProcessTimeout = std::chrono::seconds(2);
  AzureCliTestCredential const azCliCred(InfiniteCommand, options);

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
}

TEST(AzureCliCredential, ContextCancelled)
{
  AzureCliCredentialOptions options;
  options.CliProcessTimeout = std::chrono::hours(24);
  AzureCliTestCredential const azCliCred(InfiniteCommand, options);

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  auto context = Context::ApplicationContext.WithDeadline(
      std::chrono::system_clock::now() + std::chrono::hours(24));

  std::atomic<bool> thread1Started(false);

  std::thread thread1([&]() {
    thread1Started = true;
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, context)), AuthenticationException);
  });

  std::thread thread2([&]() {
    while (!thread1Started)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    context.Cancel();
  });

  thread1.join();
  thread2.join();
}

TEST(AzureCliCredential, Defaults)
{
  {
    AzureCliCredentialOptions const DefaultOptions;

    {
      AzureCliTestCredential azCliCred({});
      EXPECT_EQ(azCliCred.GetTenantId(), DefaultOptions.TenantId);
      EXPECT_EQ(azCliCred.GetCliProcessTimeout(), DefaultOptions.CliProcessTimeout);
    }

    {
      TokenCredentialOptions const options;
      AzureCliTestCredential azCliCred({}, options);
      EXPECT_EQ(azCliCred.GetTenantId(), DefaultOptions.TenantId);
      EXPECT_EQ(azCliCred.GetCliProcessTimeout(), DefaultOptions.CliProcessTimeout);
    }
  }

  {
    AzureCliCredentialOptions options;
    options.TenantId = "01234567-89AB-CDEF-0123-456789ABCDEF";
    options.CliProcessTimeout = std::chrono::seconds(12345);

    AzureCliTestCredential azCliCred({}, options);

    EXPECT_EQ(azCliCred.GetTenantId(), "01234567-89AB-CDEF-0123-456789ABCDEF");
    EXPECT_EQ(azCliCred.GetCliProcessTimeout(), std::chrono::seconds(12345));
  }
}

TEST(AzureCliCredential, CmdLine)
{
  AzureCliTestCredential azCliCred({});

  auto const cmdLineWithoutTenant
      = azCliCred.GetOriginalAzCommand("https://storage.azure.com/.default", {});

  auto const cmdLineWithTenant = azCliCred.GetOriginalAzCommand(
      "https://storage.azure.com/.default", "01234567-89AB-CDEF-0123-456789ABCDEF");

  EXPECT_EQ(
      cmdLineWithoutTenant,
      "az account get-access-token --output json --scope \"https://storage.azure.com/.default\"");

  EXPECT_EQ(
      cmdLineWithTenant,
      "az account get-access-token --output json --scope \"https://storage.azure.com/.default\""
      " --tenant \"01234567-89AB-CDEF-0123-456789ABCDEF\"");
}

TEST(AzureCliCredential, UnsafeChars)
{
  std::string const Exploit = std::string("\" | echo OWNED | ") + InfiniteCommand + " | echo \"";

  {
    AzureCliCredentialOptions options;
    options.TenantId = "01234567-89AB-CDEF-0123-456789ABCDEF";
    options.TenantId += Exploit;

    EXPECT_THROW(
        static_cast<void>(std::make_unique<AzureCliCredential>(options)), AuthenticationException);
  }

  {
    AzureCliCredentialOptions options;
    options.CliProcessTimeout = std::chrono::hours(24);
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default") + Exploit);

    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
  }
}

TEST(AzureCliCredential, StrictIso8601TimeFormat)
{
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\","
                         "\"expiresOn\":\"2022-08-24T00:43:08\"}"; // With the "T"

  AzureCliTestCredential const azCliCred(EchoCommand(Token));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");
  auto const token = azCliCred.GetToken(trc, {});

  EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  EXPECT_EQ(
      token.ExpiresOn,
      DateTime::Parse("2022-08-24T00:43:08.000000Z", DateTime::DateFormat::Rfc3339));
}
