// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/workload_identity_credential.hpp"
#include "credential_test_helper.hpp"

#include <cstdio>
#include <fstream>

#include <gtest/gtest.h>

using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::WorkloadIdentityCredential;
using Azure::Identity::WorkloadIdentityCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

namespace {
struct TempCertFile final
{
  static const char* const Path;
  ~TempCertFile();
  TempCertFile();
};
} // namespace

TEST(WorkloadIdentityCredential, GetCredentialName)
{
  TempCertFile const tempCertFile;
  WorkloadIdentityCredentialOptions options;
  options.TenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  options.ClientId = "fedcba98-7654-3210-0123-456789abcdef";
  options.TokenFilePath = TempCertFile::Path;

  WorkloadIdentityCredential const cred(options);

  EXPECT_EQ(cred.GetCredentialName(), "WorkloadIdentityCredential");
}

TEST(WorkloadIdentityCredential, GetOptionsFromEnvironment)
{
  {
    CredentialTestHelper::EnvironmentOverride const env(
        {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
         {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
         {"AZURE_AUTHORITY_HOST", ""},
         {"AZURE_FEDERATED_TOKEN_FILE", TempCertFile::Path}});

    WorkloadIdentityCredential const credDefault;
    EXPECT_EQ(credDefault.GetCredentialName(), "WorkloadIdentityCredential");

    WorkloadIdentityCredentialOptions options;
    WorkloadIdentityCredential const cred(options);
    EXPECT_EQ(cred.GetCredentialName(), "WorkloadIdentityCredential");

    EXPECT_EQ(options.TenantId, "01234567-89ab-cdef-fedc-ba8976543210");
    EXPECT_EQ(options.ClientId, "fedcba98-7654-3210-0123-456789abcdef");
    EXPECT_EQ(options.AuthorityHost, "https://login.microsoftonline.com/");
    EXPECT_EQ(options.TokenFilePath, TempCertFile::Path);
  }

  {
    std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", "foo"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    WorkloadIdentityCredentialOptions options;
    options.AuthorityHost = "bar";
    EXPECT_EQ(options.AuthorityHost, "bar");
  }

  {
    std::map<std::string, std::string> envVars
        = {{"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    WorkloadIdentityCredentialOptions options;
    EXPECT_EQ(options.AuthorityHost, "https://microsoft.com/");
  }
}

TEST(WorkloadIdentityCredential, GetOptionsFromEnvironmentInvalid)
{
  {
    CredentialTestHelper::EnvironmentOverride const env(
        {{"AZURE_TENANT_ID", ""},
         {"AZURE_CLIENT_ID", ""},
         {"AZURE_AUTHORITY_HOST", ""},
         {"AZURE_FEDERATED_TOKEN_FILE", ""}});

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    WorkloadIdentityCredential const credDefault;
    EXPECT_THROW(credDefault.GetToken(trc, {}), AuthenticationException);
    WorkloadIdentityCredentialOptions options;
    WorkloadIdentityCredential const cred(options);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // The http scheme is not supported.
  {
    CredentialTestHelper::EnvironmentOverride const env(
        {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
         {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
         {"AZURE_AUTHORITY_HOST", "http://microsoft.com/"},
         {"AZURE_FEDERATED_TOKEN_FILE", TempCertFile::Path}});

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    WorkloadIdentityCredential const credDefault;
    EXPECT_THROW(credDefault.GetToken(trc, {}), AuthenticationException);
    WorkloadIdentityCredentialOptions options;
    WorkloadIdentityCredential const cred(options);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);

    try
    {
      auto const token = cred.GetToken(trc, {});
    }
    catch (AuthenticationException const& e)
    {
      EXPECT_TRUE(std::string(e.what()).find("https") != std::string::npos) << e.what();
    }
  }
}

TEST(WorkloadIdentityCredential, Regular)
{
  TempCertFile const tempCertFile;

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        WorkloadIdentityCredentialOptions options;
        options.Transport.Transport = transport;
        options.TenantId = "01234567-89ab-cdef-fedc-ba8976543210";
        options.ClientId = "fedcba98-7654-3210-0123-456789abcdef";
        options.TokenFilePath = TempCertFile::Path;

        return std::make_unique<WorkloadIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion="; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion="; // cspell:enable

    EXPECT_GT(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_GT(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_GT(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_GT(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(WorkloadIdentityCredential, AzureStack)
{
  TempCertFile const tempCertFile;

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        WorkloadIdentityCredentialOptions options;
        options.Transport.Transport = transport;
        options.TenantId = "adfs";
        options.ClientId = "fedcba98-7654-3210-0123-456789abcdef";
        options.TokenFilePath = TempCertFile::Path;

        return std::make_unique<WorkloadIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

  EXPECT_EQ(request1.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com"
          "&client_assertion="; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion="; // cspell:enable

    EXPECT_GT(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_GT(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_GT(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_GT(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(WorkloadIdentityCredential, Authority)
{
  TempCertFile const tempCertFile;

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        WorkloadIdentityCredentialOptions options;
        options.Transport.Transport = transport;
        options.TenantId = "01234567-89ab-cdef-fedc-ba8976543210";
        options.ClientId = "fedcba98-7654-3210-0123-456789abcdef";
        options.AuthorityHost = "https://microsoft.com/";
        options.TokenFilePath = TempCertFile::Path;

        return std::make_unique<WorkloadIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion="; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion="; // cspell:enable

    EXPECT_GT(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_GT(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_GT(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_GT(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

namespace {
const char* const TempCertFile::Path = "azure-identity-test.pem";

TempCertFile::~TempCertFile() { std::remove(Path); }

TempCertFile::TempCertFile()
{
  std::ofstream cert(Path, std::ios_base::out | std::ios_base::trunc);

  // The file contents is the following text, encoded as base64:
  // "Base64 encoded JSON text to simulate a client assertion"
  cert << // cspell:disable
      "QmFzZTY0IGVuY29kZWQgSlNPTiB0ZXh0IHRvIHNpbXVsYXRlIGEgY2xpZW50IGFzc2VydGlvbg==\n";
  // cspell:enable
}
} // namespace
