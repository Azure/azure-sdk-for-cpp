// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/azure_cli_credential.hpp"

#include <azure/core/diagnostics/logger.hpp>
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
  int m_localTimeToUtcDiffSeconds = 0;

  std::string GetAzCommand(std::string const& resource, std::string const& tenantId) const override
  {
    static_cast<void>(resource);
    static_cast<void>(tenantId);

    return m_command;
  }

  int GetLocalTimeToUtcDiffSeconds() const override { return m_localTimeToUtcDiffSeconds; }

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

  void SetLocalTimeToUtcDiffSeconds(int diff) { m_localTimeToUtcDiffSeconds = diff; }
};
} // namespace

#if !defined(AZ_PLATFORM_WINDOWS) \
    || (!defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP) // not UWP
TEST(AzureCliCredential, Success)
#else
TEST(AzureCliCredential, NotAvailable)
#endif
{
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\","
                         "\"expiresOn\":\"2022-08-24 00:43:08.000000\","
                         "\"tenant\":\"72f988bf-86f1-41af-91ab-2d7cd011db47\","
                         "\"tokenType\":\"Bearer\"}";

  AzureCliTestCredential const azCliCred(EchoCommand(Token));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");
#if !defined(AZ_PLATFORM_WINDOWS) \
    || (!defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP) // not UWP
  auto const token = azCliCred.GetToken(trc, {});

  EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  EXPECT_EQ(
      token.ExpiresOn,
      DateTime::Parse("2022-08-24T00:43:08.000000Z", DateTime::DateFormat::Rfc3339));
#else // UWP
  // The credential should throw during GetToken() and not during construction, because it allows
  // customers to put it into ChainedTokenCredential and successfully use it there without writing
  // ifdefs for UWP. It is not too late to throw - for example, if Azure CLI is not installed, then
  // the credential will also find out during GetToken() and not during construction (if we had to
  // find out during the construction, we'd have to fire up some 'az' command in constructor; again,
  // that would also make it hard to put the credential into ChainedTokenCredential).
  EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
#endif // UWP
}

#if !defined(AZ_PLATFORM_WINDOWS) \
    || (!defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP) // not UWP
TEST(AzureCliCredential, Error)
{
  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Informational);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  AzureCliTestCredential const azCliCred(
      EchoCommand("ERROR: Please run az login to setup account."));

  EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
  EXPECT_EQ(log[0].first, Logger::Level::Informational);
  EXPECT_EQ(
      log[0].second,
      "Identity: AzureCliCredential created."
      "\nSuccessful creation does not guarantee further successful token retrieval.");

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  log.clear();
  auto const errorMsg = "Identity: AzureCliCredential didn't get the token:"
                        " \"ERROR: Please run az login to setup account."
#if defined(AZ_PLATFORM_WINDOWS)
                        "\r"
#endif
                        "\n\"";

  EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
  EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
  EXPECT_EQ(log[0].first, Logger::Level::Warning);
  EXPECT_EQ(log[0].second, errorMsg);

  Logger::SetListener(nullptr);
}

TEST(AzureCliCredential, GetCredentialName)
{
  AzureCliTestCredential const cred(EmptyOutputCommand);
  EXPECT_EQ(cred.GetCredentialName(), "AzureCliCredential");
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

TEST(AzureCliCredential, ExpiresOnUnixTimestampInt)
{
  // 'expires_on' is 1700692424, which is a Unix timestamp of a date in 2023.
  // 'ExpiresOn' is a date in 2022.
  // The test checks that when both are present, 'expires_on' value (2023) is taken,
  // and not that of 'ExpiresOn'.
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\","
                         "\"expiresOn\":\"2022-08-24 00:43:08.000000\"," // <-- 2022
                         "\"expires_on\":1700692424}"; // <-- 2023

  AzureCliTestCredential const azCliCred(EchoCommand(Token));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  auto const token = azCliCred.GetToken(trc, {});

  EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  EXPECT_EQ(
      token.ExpiresOn,
      DateTime::Parse("2023-11-22T22:33:44.000000Z", DateTime::DateFormat::Rfc3339));
}

TEST(AzureCliCredential, ExpiresOnUnixTimestampString)
{
  // 'expires_on' is 1700692424, which is a Unix timestamp of a date in 2023.
  // 'expiresOn' is a date in 2022.
  // The test checks that when both are present, 'expires_on' value (2023) is taken,
  // and not that of 'expiresOn'.
  // The test is similar to the one above, but the Unix timestamp is represented as string
  // containing an integer.
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\","
                         "\"expiresOn\":\"2022-08-24 00:43:08.000000\"," // <-- 2022
                         "\"expires_on\":\"1700692424\"}"; // <-- 2023

  AzureCliTestCredential const azCliCred(EchoCommand(Token));

  TokenRequestContext trc;
  trc.Scopes.push_back("https://storage.azure.com/.default");

  auto const token = azCliCred.GetToken(trc, {});

  EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  EXPECT_EQ(
      token.ExpiresOn,
      DateTime::Parse("2023-11-22T22:33:44.000000Z", DateTime::DateFormat::Rfc3339));
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
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default"));
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);
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

class ParameterizedTestForDisallowedChars : public ::testing::TestWithParam<std::string> {
protected:
  std::string value;
};

TEST_P(ParameterizedTestForDisallowedChars, DisallowedCharsForScopeAndTenantId)
{
  std::string const InvalidValue = GetParam();

  // Tenant ID test via AzureCliCredentialOptions directly.
  {
    AzureCliCredentialOptions options;
    options.TenantId = "01234567-89AB-CDEF-0123-456789ABCDEF";
    options.TenantId += InvalidValue;
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default"));
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);

    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") != std::string::npos) << e.what();
    }
  }

  // Tenant ID test via TokenRequestContext, using a wildcard for AdditionallyAllowedTenants.
  {
    AzureCliCredentialOptions options;
    options.CliProcessTimeout = std::chrono::hours(24);
    options.AdditionallyAllowedTenants.push_back("*");
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default"));
    trc.TenantId = InvalidValue;
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);

    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") != std::string::npos) << e.what();
    }
  }

  // Tenant ID test via TokenRequestContext, using a specific AdditionallyAllowedTenants value.
  {
    AzureCliCredentialOptions options;
    options.AdditionallyAllowedTenants.push_back(InvalidValue);
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default"));
    trc.TenantId = InvalidValue;
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);

    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") != std::string::npos) << e.what();
    }
  }

  // Scopes test via TokenRequestContext.
  {
    AzureCliCredentialOptions options;
    options.CliProcessTimeout = std::chrono::hours(24);
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default") + InvalidValue);
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);

    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") != std::string::npos) << e.what();
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    AzureCliCredential,
    ParameterizedTestForDisallowedChars,
    ::testing::Values(" ", "|", "`", "\"", "'", ";", "&"));

class ParameterizedTestForCharDifferences : public ::testing::TestWithParam<std::string> {
protected:
  std::string value;
};

TEST_P(ParameterizedTestForCharDifferences, ValidCharsForScopeButNotTenantId)
{
  std::string const ValidScopeButNotTenantId = GetParam();

  {
    AzureCliCredentialOptions options;
    options.TenantId = "01234567-89AB-CDEF-0123-456789ABCDEF";
    options.TenantId += ValidScopeButNotTenantId;
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default"));
    EXPECT_THROW(static_cast<void>(azCliCred.GetToken(trc, {})), AuthenticationException);

    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") != std::string::npos) << e.what();
    }
  }

  {
    AzureCliCredentialOptions options;
    options.CliProcessTimeout = std::chrono::hours(24);
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(
        std::string("https://storage.azure.com/.default") + ValidScopeButNotTenantId);

    // We expect the GetToken to fail, but not because of the unsafe chars.
    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") == std::string::npos) << e.what();
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    AzureCliCredential,
    ParameterizedTestForCharDifferences,
    ::testing::Values(":", "/", "_"));

class ParameterizedTestForAllowedChars : public ::testing::TestWithParam<std::string> {
protected:
  std::string value;
};

TEST_P(ParameterizedTestForAllowedChars, ValidCharsForScopeAndTenantId)
{
  std::string const ValidChars = GetParam();

  {
    AzureCliCredentialOptions options;
    options.TenantId = "01234567-89AB-CDEF-0123-456789ABCDEF";
    options.TenantId += ValidChars;
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default"));

    // We expect the GetToken to fail, but not because of the unsafe chars.
    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") == std::string::npos) << e.what();
    }
  }

  {
    AzureCliCredentialOptions options;
    options.CliProcessTimeout = std::chrono::hours(24);
    AzureCliCredential azCliCred(options);

    TokenRequestContext trc;
    trc.Scopes.push_back(std::string("https://storage.azure.com/.default") + ValidChars);

    // We expect the GetToken to fail, but not because of the unsafe chars.
    try
    {
      auto const token = azCliCred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("Unsafe") == std::string::npos) << e.what();
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    AzureCliCredential,
    ParameterizedTestForAllowedChars,
    ::testing::Values(".", "-", "A", "9"));

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

TEST(AzureCliCredential, LocalTime)
{
  constexpr auto Token = "{\"accessToken\":\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\","
                         "\"expiresOn\":\"2023-12-07 00:43:08\"}";

  {
    AzureCliTestCredential azCliCred(EchoCommand(Token));
    azCliCred.SetLocalTimeToUtcDiffSeconds(-28800); // Redmond (no DST)

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");
    auto const token = azCliCred.GetToken(trc, {});

    EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    EXPECT_EQ(
        token.ExpiresOn, DateTime::Parse("2023-12-07T08:43:08Z", DateTime::DateFormat::Rfc3339));
  }

  {
    AzureCliTestCredential azCliCred(EchoCommand(Token));
    azCliCred.SetLocalTimeToUtcDiffSeconds(7200); // Kyiv (no DST)

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");
    auto const token = azCliCred.GetToken(trc, {});

    EXPECT_EQ(token.Token, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    EXPECT_EQ(
        token.ExpiresOn, DateTime::Parse("2023-12-06T22:43:08Z", DateTime::DateFormat::Rfc3339));
  }
}

TEST(AzureCliCredential, Diagnosability)
{
  {
    AzureCliTestCredential const azCliCred(
        EchoCommand("az is not recognized as an internal or external command, "
                    "operable program or batch file."));

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");
    try
    {
      static_cast<void>(azCliCred.GetToken(trc, {}));
    }
    catch (AuthenticationException const& e)
    {
      std::string const expectedMsgStart
          = "AzureCliCredential didn't get the token: "
            "\"az is not recognized as an internal or external command, "
            "operable program or batch file.";

      std::string actualMsgStart = e.what();
      actualMsgStart.resize(expectedMsgStart.length());

      // It is enough to compare StartsWith() and not deal with
      // the entire string due to '/n' and '/r/n' differences.
      EXPECT_EQ(actualMsgStart, expectedMsgStart);
    }
  }

  {
    AzureCliTestCredential const azCliCred(EchoCommand("{\"property\":\"value\"}"));

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");
    try
    {
      static_cast<void>(azCliCred.GetToken(trc, {}));
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_EQ(
          e.what(),
          std::string("AzureCliCredential didn't get the token: "
                      "\"Token JSON object: can't find or parse 'accessToken' property.\n"
                      "See Azure::Core::Diagnostics::Logger for details "
                      "(https://aka.ms/azsdk/cpp/identity/troubleshooting).\""));
    }
  }
}
#endif // not UWP
