// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/storage/files/datalake/datalake_directory_client.hpp>
#include <azure/storage/files/datalake/datalake_file_client.hpp>
#include <azure/storage/files/datalake/datalake_file_system_client.hpp>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Test {
  namespace {
    class NoTokenCredential final : public Core::Credentials::TokenCredential {
    public:
      NoTokenCredential() : TokenCredential("NoTokenCredential") {}

      mutable int TokenRequests = 0;

      Core::Credentials::AccessToken GetToken(
          Core::Credentials::TokenRequestContext const&,
          Core::Context const&) const override
      {
        ++TokenRequests;
        throw std::runtime_error("client construction must not request a token");
      }
    };

    class CountingTokenCredential final : public Core::Credentials::TokenCredential {
    public:
      CountingTokenCredential() : TokenCredential("CountingTokenCredential") {}

      mutable int TokenRequests = 0;
      mutable std::vector<std::string> TenantIds;

      Core::Credentials::AccessToken GetToken(
          Core::Credentials::TokenRequestContext const& tokenRequestContext,
          Core::Context const&) const override
      {
        ++TokenRequests;
        TenantIds.emplace_back(tokenRequestContext.TenantId);
        Core::Credentials::AccessToken token;
        token.Token = "test-token";
        token.ExpiresOn = DateTime::clock::now() + std::chrono::hours(1);
        return token;
      }
    };

    class RecordingTransport final : public Core::Http::HttpTransport {
    public:
      std::vector<std::string> RequestHosts;

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Context const&) override
      {
        RequestHosts.emplace_back(request.GetUrl().GetHost());
        throw std::runtime_error("request recorded");
      }
    };

    class ChallengeTransport final : public Core::Http::HttpTransport {
    public:
      std::vector<std::string> RequestHosts;
      std::vector<bool> HasAuthorization;

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Context const&) override
      {
        RequestHosts.emplace_back(request.GetUrl().GetHost());
        HasAuthorization.emplace_back(request.GetHeader("Authorization").HasValue());
        if (RequestHosts.size() == 1)
        {
          auto response = std::make_unique<Core::Http::RawResponse>(
              1, 1, Core::Http::HttpStatusCode::Unauthorized, "Unauthorized");
          response->SetHeader(
              "WWW-Authenticate",
              "Bearer authorization_uri=\"https://login.microsoftonline.com/"
              "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/authorize\"");
          response->SetBodyStream(std::make_unique<Core::IO::MemoryBodyStream>(nullptr, 0));
          return response;
        }
        throw std::runtime_error("authenticated request recorded");
      }
    };

    class RenameTransport final : public Core::Http::HttpTransport {
    public:
      std::vector<std::string> RequestHosts;

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Context const&) override
      {
        RequestHosts.emplace_back(request.GetUrl().GetHost());
        auto response = std::make_unique<Core::Http::RawResponse>(
            1, 1, Core::Http::HttpStatusCode::Created, "Created");
        response->SetHeader("ETag", "\"test-etag\"");
        response->SetHeader("Last-Modified", "Wed, 23 Oct 2019 23:49:13 GMT");
        response->SetBodyStream(std::make_unique<Core::IO::MemoryBodyStream>(nullptr, 0));
        return response;
      }
    };

    constexpr auto OneLakeServiceUrl = "https://onelake.dfs.fabric.microsoft.com";
    constexpr auto ExpectedWorkspaceUrl
        = "https://1234567890abcdef1234567890abcdef.z12.blob.fabric.microsoft.com/"
          "12345678-90ab-cdef-1234-567890abcdef";

    std::string GetExpectedWorkspaceUrl(
        const std::string& cloudDomain,
        const std::string& ring,
        const std::string& workspaceId)
    {
      return "https://1234567890abcdef1234567890abcdef.z12." + ring + "blob." + cloudDomain + "/"
          + workspaceId;
    }
  } // namespace

  TEST(OneLakeWorkspaceClientTest, CreatesClientFromHyphenatedWorkspaceId)
  {
    auto credential = std::make_shared<NoTokenCredential>();

    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        OneLakeServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential);

    EXPECT_EQ(ExpectedWorkspaceUrl, client.GetUrl());
  }

  TEST(OneLakeWorkspaceClientTest, CreatesClientFromCompactWorkspaceId)
  {
    auto credential = std::make_shared<NoTokenCredential>();

    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        OneLakeServiceUrl, "1234567890abcdef1234567890abcdef", credential);

    EXPECT_EQ(
        "https://1234567890abcdef1234567890abcdef.z12.blob.fabric.microsoft.com/"
        "1234567890abcdef1234567890abcdef",
        client.GetUrl());
  }

  TEST(OneLakeWorkspaceClientTest, ConstructionDoesNotRequestToken)
  {
    auto credential = std::make_shared<NoTokenCredential>();

    DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        OneLakeServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential);

    EXPECT_EQ(0, credential->TokenRequests);
  }

  TEST(OneLakeWorkspaceClientTest, NormalizesSharedAndApiEndpointsAcrossCloudsAndRings)
  {
    struct RingCase final
    {
      std::string InputPrefix;
      std::string OutputPrefix;
    };
    const std::vector<std::string> cloudDomains{
        "fabric.microsoft.com",
        "fabric-df.microsoft.com",
        "fabric.microsoft.us",
        "fabric.sovcloud-api.fr"};
    const std::vector<RingCase> rings{
        {"", ""},
        {"daily-", "daily-"},
        {"i-daily-", "daily-"},
        {"dxt-", "dxt-"},
        {"i-dxt-", "dxt-"},
        {"msit-", "msit-"},
        {"i-msit-", "msit-"}};
    auto credential = std::make_shared<NoTokenCredential>();

    for (const auto& cloudDomain : cloudDomains)
    {
      for (const auto& ring : rings)
      {
        const auto expectedUrl = GetExpectedWorkspaceUrl(
            cloudDomain, ring.OutputPrefix, "12345678-90ab-cdef-1234-567890abcdef");
        const std::vector<std::string> serviceUrls{
            "https://" + ring.InputPrefix + "onelake.dfs." + cloudDomain,
            "https://" + ring.InputPrefix + "onelake.blob." + cloudDomain,
            "https://" + ring.InputPrefix + "api.onelake." + cloudDomain};

        for (const auto& serviceUrl : serviceUrls)
        {
          SCOPED_TRACE(serviceUrl);
          auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential);
          EXPECT_EQ(expectedUrl, client.GetUrl());
        }
      }
    }
  }

  TEST(OneLakeWorkspaceClientTest, RemovesRegionalPrefixes)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> serviceUrls{
        "https://westus-onelake.dfs.fabric.microsoft.com",
        "https://westus-onelake.blob.fabric.microsoft.com",
        "https://westus-api.onelake.fabric.microsoft.com",
        "https://future-region-9-onelake.dfs.fabric.microsoft.com"};

    for (const auto& serviceUrl : serviceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
          serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential);
      EXPECT_EQ(ExpectedWorkspaceUrl, client.GetUrl());
    }
  }

  TEST(OneLakeWorkspaceClientTest, EnforcesDnsLengthLimits)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::string maximumDfsRegion(55, 'a');
    const std::string overlongDfsRegion(56, 'a');
    const std::string maximumApiRegion(59, 'a');
    const std::string overlongApiRegion(60, 'a');
    const std::vector<std::string> acceptedServiceUrls{
        "https://" + maximumDfsRegion + "-onelake.dfs.fabric.microsoft.com",
        "https://" + maximumDfsRegion + "-onelake.blob.fabric.microsoft.com",
        "https://" + maximumApiRegion + "-api.onelake.fabric.microsoft.com"};
    const std::vector<std::string> rejectedServiceUrls{
        "https://" + overlongDfsRegion + "-onelake.dfs.fabric.microsoft.com",
        "https://" + overlongDfsRegion + "-onelake.blob.fabric.microsoft.com",
        "https://" + overlongApiRegion + "-api.onelake.fabric.microsoft.com",
        "https://a-onelake.dfs."
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa."
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa."
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa."
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.com"};

    for (const auto& serviceUrl : acceptedServiceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      EXPECT_NO_THROW(DataLakeFileSystemClient::CreateForOneLakeWorkspace(
          serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential));
    }
    for (const auto& serviceUrl : rejectedServiceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      EXPECT_THROW(
          DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential),
          std::invalid_argument);
    }
  }

  TEST(OneLakeWorkspaceClientTest, PreservesRingBeforeRegionalPrefix)
  {
    struct EndpointCase final
    {
      std::string ServiceUrl;
      std::string Ring;
    };
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<EndpointCase> cases{
        {"https://daily-eastus2euap-api.onelake.fabric.microsoft.com", "daily-"},
        {"https://i-daily-eastus2euap-api.onelake.fabric.microsoft.com", "daily-"},
        {"https://dxt-westus-onelake.dfs.fabric.microsoft.com", "dxt-"},
        {"https://i-msit-francecentral-onelake.blob.fabric.microsoft.com", "msit-"}};

    for (const auto& endpointCase : cases)
    {
      SCOPED_TRACE(endpointCase.ServiceUrl);
      auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
          endpointCase.ServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential);
      EXPECT_EQ(
          GetExpectedWorkspaceUrl(
              "fabric.microsoft.com", endpointCase.Ring, "12345678-90ab-cdef-1234-567890abcdef"),
          client.GetUrl());
    }
  }

  TEST(OneLakeWorkspaceClientTest, NormalizesCanonicalWorkspaceEndpoints)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> rings{"", "daily-", "dxt-", "msit-"};

    for (const auto& ring : rings)
    {
      const auto expectedUrl = GetExpectedWorkspaceUrl(
          "fabric.microsoft.com", ring, "12345678-90ab-cdef-1234-567890abcdef");
      const std::vector<std::string> serviceUrls{
          "https://1234567890abcdef1234567890abcdef.z12." + ring + "dfs.fabric.microsoft.com",
          "https://1234567890abcdef1234567890abcdef.z12." + ring + "blob.fabric.microsoft.com"};

      for (const auto& serviceUrl : serviceUrls)
      {
        SCOPED_TRACE(serviceUrl);
        auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
            serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential);
        EXPECT_EQ(expectedUrl, client.GetUrl());
      }
    }
  }

  TEST(OneLakeWorkspaceClientTest, NormalizesCaseAndEncodesChildUrl)
  {
    auto credential = std::make_shared<NoTokenCredential>();

    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        "HTTPS://1234567890ABCDEF1234567890ABCDEF.Z12.DAILY-DFS.FABRIC.MICROSOFT.COM",
        "12345678-90AB-CDEF-1234-567890ABCDEF",
        credential);
    auto directoryClient = client.GetDirectoryClient("item id/Files/landing #1?");

    EXPECT_EQ(
        "https://1234567890abcdef1234567890abcdef.z12.daily-blob.fabric.microsoft.com/"
        "12345678-90AB-CDEF-1234-567890ABCDEF/item%20id/Files/landing%20%231%3F",
        directoryClient.GetUrl());
  }

  TEST(OneLakeWorkspaceClientTest, RejectsMalformedRegionalPrefixes)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> serviceUrls{
        "https://-onelake.dfs.fabric.microsoft.com",
        "https://westus--onelake.dfs.fabric.microsoft.com",
        "https://west--us-onelake.dfs.fabric.microsoft.com",
        "https://-api.onelake.fabric.microsoft.com",
        "https://westus--api.onelake.fabric.microsoft.com",
        "https://west--us-api.onelake.fabric.microsoft.com"};

    for (const auto& serviceUrl : serviceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      EXPECT_THROW(
          DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential),
          std::invalid_argument);
    }
  }

  TEST(OneLakeWorkspaceClientTest, AcceptsRootPath)
  {
    auto credential = std::make_shared<NoTokenCredential>();

    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        "https://onelake.dfs.fabric.microsoft.com/",
        "12345678-90ab-cdef-1234-567890abcdef",
        credential);

    EXPECT_EQ(ExpectedWorkspaceUrl, client.GetUrl());
  }

  TEST(OneLakeWorkspaceClientTest, RejectsUnsafeServiceUrls)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> serviceUrls{
        "http://onelake.dfs.fabric.microsoft.com",
        "https://user@onelake.dfs.fabric.microsoft.com",
        "https://user:password@onelake.dfs.fabric.microsoft.com",
        "https://onelake.dfs.fabric.microsoft.com?api-version=1",
        "https://onelake.dfs.fabric.microsoft.com#fragment",
        "https://onelake.dfs.fabric.microsoft.com/workspace",
        "https://onelake.dfs.fabric.microsoft.com//",
        "https://onelake.dfs.fabric.microsoft.com:443",
        "https://onelake.dfs.fabric.microsoft.com:0",
        "https://onelake.dfs.fabric.microsoft.com.",
        "https://onelake..dfs.fabric.microsoft.com",
        "https://onelake.dfs..fabric.microsoft.com",
        "https://127.0.0.1",
        "https://[::1]",
        "https://onelake.dfs.fabric.microsoft.com.attacker.example",
        "https://onelake.dfs.evilfabric.microsoft.com",
        "https://api.onelake.fabric.microsoft.example"};

    for (const auto& serviceUrl : serviceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      EXPECT_THROW(
          DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential),
          std::invalid_argument);
    }
  }

  TEST(OneLakeWorkspaceClientTest, RejectsMalformedWorkspaceIds)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> workspaceIds{
        "",
        "1234567890abcdef1234567890abcde",
        "1234567890abcdef1234567890abcdef0",
        "1234567890abcdef1234567890abcdeg",
        "1234567-890ab-cdef-1234-567890abcdef",
        "12345678-90ab-cdef-1234-567890abcde",
        "12345678_90ab-cdef-1234-567890abcdef",
        "{12345678-90ab-cdef-1234-567890abcdef}"};

    for (const auto& workspaceId : workspaceIds)
    {
      SCOPED_TRACE(workspaceId);
      EXPECT_THROW(
          DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              OneLakeServiceUrl, workspaceId, credential),
          std::invalid_argument);
    }
  }

  TEST(OneLakeWorkspaceClientTest, RejectsInvalidWorkspaceEndpoints)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> serviceUrls{
        "https://aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.z12.dfs.fabric.microsoft.com",
        "https://1234567890abcdef1234567890abcdef.z13.dfs.fabric.microsoft.com",
        "https://12345678-90ab-cdef-1234-567890abcdef.z12.dfs.fabric.microsoft.com",
        "https://1234567890abcdef1234567890abcdef.z12.i-daily-dfs.fabric.microsoft.com",
        "https://1234567890abcdef1234567890abcdef.z12.i-dxt-blob.fabric.microsoft.com",
        "https://1234567890abcdef1234567890abcdef.z12.future-dfs.fabric.microsoft.com"};

    for (const auto& serviceUrl : serviceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      EXPECT_THROW(
          DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential),
          std::invalid_argument);
    }
  }

  TEST(OneLakeWorkspaceClientTest, RejectsDedicatedEndpoints)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    const std::vector<std::string> serviceUrls{
        "https://onelake.pbidedicated.windows.net",
        "https://onelake.pbidedicated.windows-int.net",
        "https://onelake.dfs.pbidedicated.windows.net",
        "https://onelake.blob.pbidedicated.windows-int.net"};

    for (const auto& serviceUrl : serviceUrls)
    {
      SCOPED_TRACE(serviceUrl);
      EXPECT_THROW(
          DataLakeFileSystemClient::CreateForOneLakeWorkspace(
              serviceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential),
          std::invalid_argument);
    }
  }

  TEST(OneLakeWorkspaceClientTest, RejectsNullCredential)
  {
    std::shared_ptr<const Core::Credentials::TokenCredential> credential;

    EXPECT_THROW(
        DataLakeFileSystemClient::CreateForOneLakeWorkspace(
            OneLakeServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential),
        std::invalid_argument);
  }

  TEST(OneLakeWorkspaceClientTest, RejectsUnsafeClientOptions)
  {
    auto credential = std::make_shared<NoTokenCredential>();

    DataLakeClientOptions secondaryHostOptions;
    secondaryHostOptions.SecondaryHostForRetryReads = "attacker.example";
    EXPECT_THROW(
        DataLakeFileSystemClient::CreateForOneLakeWorkspace(
            OneLakeServiceUrl,
            "12345678-90ab-cdef-1234-567890abcdef",
            credential,
            secondaryHostOptions),
        std::invalid_argument);

    DataLakeClientOptions customerProvidedKeyOptions;
    EncryptionKey customerProvidedKey;
    customerProvidedKeyOptions.CustomerProvidedKey = customerProvidedKey;
    EXPECT_THROW(
        DataLakeFileSystemClient::CreateForOneLakeWorkspace(
            OneLakeServiceUrl,
            "12345678-90ab-cdef-1234-567890abcdef",
            credential,
            customerProvidedKeyOptions),
        std::invalid_argument);

    DataLakeClientOptions audienceOptions;
    audienceOptions.Audience = DataLakeAudience("https://attacker.example/");
    EXPECT_THROW(
        DataLakeFileSystemClient::CreateForOneLakeWorkspace(
            OneLakeServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential, audienceOptions),
        std::invalid_argument);

    EXPECT_EQ(0, credential->TokenRequests);
  }

  TEST(OneLakeWorkspaceClientTest, AcceptsExplicitDefaultAudience)
  {
    auto credential = std::make_shared<NoTokenCredential>();
    DataLakeClientOptions options;
    options.Audience = DataLakeAudience::DefaultAudience;

    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        OneLakeServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential, options);

    EXPECT_EQ(ExpectedWorkspaceUrl, client.GetUrl());
    EXPECT_EQ(0, credential->TokenRequests);
  }

  TEST(OneLakeWorkspaceClientTest, RoutesDfsAndBlobRequestsToWorkspaceHosts)
  {
    auto credential = std::make_shared<CountingTokenCredential>();
    auto transport = std::make_shared<RecordingTransport>();
    DataLakeClientOptions options;
    options.Retry.MaxRetries = 0;
    options.Transport.Transport = transport;
    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        "https://daily-onelake.dfs.fabric.microsoft.com",
        "12345678-90ab-cdef-1234-567890abcdef",
        credential,
        options);
    EXPECT_EQ(0, credential->TokenRequests);

    EXPECT_THROW(client.GetDirectoryClient("item/Files/landing").Create(), std::runtime_error);
    EXPECT_THROW(client.GetProperties(), std::runtime_error);

    ASSERT_EQ(2U, transport->RequestHosts.size());
    EXPECT_EQ(
        "1234567890abcdef1234567890abcdef.z12.daily-dfs.fabric.microsoft.com",
        transport->RequestHosts[0]);
    EXPECT_EQ(
        "1234567890abcdef1234567890abcdef.z12.daily-blob.fabric.microsoft.com",
        transport->RequestHosts[1]);
  }

  TEST(OneLakeWorkspaceClientTest, RenameResultsRetainWorkspaceBlobHostAcrossRings)
  {
    struct RingCase final
    {
      std::string ServicePrefix;
      std::string OutputPrefix;
    };
    const std::vector<RingCase> rings{
        {"", ""}, {"daily-", "daily-"}, {"dxt-", "dxt-"}, {"msit-", "msit-"}};

    for (const auto& ring : rings)
    {
      auto credential = std::make_shared<CountingTokenCredential>();
      auto transport = std::make_shared<RenameTransport>();
      DataLakeClientOptions options;
      options.Retry.MaxRetries = 0;
      options.Transport.Transport = transport;
      auto fileSystemClient = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
          "https://" + ring.ServicePrefix + "onelake.dfs.fabric.microsoft.com",
          "12345678-90ab-cdef-1234-567890abcdef",
          credential,
          options);
      auto directoryClient = fileSystemClient.GetDirectoryClient("item/Files/source");
      const std::string expectedPrefix = "https://1234567890abcdef1234567890abcdef.z12."
          + ring.OutputPrefix + "blob.fabric.microsoft.com/12345678-90ab-cdef-1234-567890abcdef/";

      SCOPED_TRACE(ring.ServicePrefix);
      auto renamedFile
          = fileSystemClient.RenameFile("source-file", "item/Files/filesystem-file").Value;
      EXPECT_EQ(expectedPrefix + "item/Files/filesystem-file", renamedFile.GetUrl());
      EXPECT_ANY_THROW(renamedFile.GetProperties());

      auto renamedDirectory
          = fileSystemClient.RenameDirectory("source-directory", "item/Files/filesystem-directory")
                .Value;
      EXPECT_EQ(expectedPrefix + "item/Files/filesystem-directory", renamedDirectory.GetUrl());
      EXPECT_ANY_THROW(renamedDirectory.GetProperties());

      auto directoryRenamedFile
          = directoryClient.RenameFile("source-file", "item/Files/directory-file").Value;
      EXPECT_EQ(expectedPrefix + "item/Files/directory-file", directoryRenamedFile.GetUrl());
      EXPECT_ANY_THROW(directoryRenamedFile.GetProperties());

      auto directoryRenamedSubdirectory
          = directoryClient
                .RenameSubdirectory("source-directory", "item/Files/directory-subdirectory")
                .Value;
      EXPECT_EQ(
          expectedPrefix + "item/Files/directory-subdirectory",
          directoryRenamedSubdirectory.GetUrl());
      EXPECT_ANY_THROW(directoryRenamedSubdirectory.GetProperties());

      ASSERT_EQ(8U, transport->RequestHosts.size());
      for (std::size_t index = 0; index < transport->RequestHosts.size(); index += 2)
      {
        EXPECT_EQ(
            "1234567890abcdef1234567890abcdef.z12." + ring.OutputPrefix
                + "dfs.fabric.microsoft.com",
            transport->RequestHosts[index]);
        EXPECT_EQ(
            "1234567890abcdef1234567890abcdef.z12." + ring.OutputPrefix
                + "blob.fabric.microsoft.com",
            transport->RequestHosts[index + 1]);
      }
    }
  }

  TEST(OneLakeWorkspaceClientTest, KeepsDisabledTenantDiscoveryOnWorkspaceBlobHost)
  {
    auto credential = std::make_shared<CountingTokenCredential>();
    auto transport = std::make_shared<ChallengeTransport>();
    DataLakeClientOptions options;
    options.EnableTenantDiscovery = false;
    options.Retry.MaxRetries = 0;
    options.Transport.Transport = transport;
    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        OneLakeServiceUrl, "12345678-90ab-cdef-1234-567890abcdef", credential, options);

    EXPECT_ANY_THROW(client.GetProperties());

    ASSERT_EQ(1U, transport->RequestHosts.size());
    EXPECT_EQ(
        "1234567890abcdef1234567890abcdef.z12.blob.fabric.microsoft.com",
        transport->RequestHosts[0]);
    EXPECT_TRUE(transport->HasAuthorization[0]);
    ASSERT_EQ(1U, credential->TenantIds.size());
    EXPECT_TRUE(credential->TenantIds[0].empty());
  }

  TEST(OneLakeWorkspaceClientTest, KeepsTenantDiscoveryChallengeOnWorkspaceDfsHost)
  {
    auto credential = std::make_shared<CountingTokenCredential>();
    auto transport = std::make_shared<ChallengeTransport>();
    DataLakeClientOptions options;
    options.EnableTenantDiscovery = true;
    options.Retry.MaxRetries = 0;
    options.Transport.Transport = transport;
    auto client = DataLakeFileSystemClient::CreateForOneLakeWorkspace(
        "https://daily-api.onelake.fabric.microsoft.com",
        "12345678-90ab-cdef-1234-567890abcdef",
        credential,
        options);

    EXPECT_THROW(client.GetDirectoryClient("item/Files/landing").Create(), std::runtime_error);

    ASSERT_EQ(2U, transport->RequestHosts.size());
    EXPECT_EQ(
        "1234567890abcdef1234567890abcdef.z12.daily-dfs.fabric.microsoft.com",
        transport->RequestHosts[0]);
    EXPECT_EQ(transport->RequestHosts[0], transport->RequestHosts[1]);
    EXPECT_FALSE(transport->HasAuthorization[0]);
    EXPECT_TRUE(transport->HasAuthorization[1]);
    ASSERT_EQ(1U, credential->TenantIds.size());
    EXPECT_EQ("72f988bf-86f1-41af-91ab-2d7cd011db47", credential->TenantIds[0]);
  }

}}}}} // namespace Azure::Storage::Files::DataLake::Test