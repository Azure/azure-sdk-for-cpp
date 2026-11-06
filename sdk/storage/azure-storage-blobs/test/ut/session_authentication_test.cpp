// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "blob_container_client_test.hpp"

#include <azure/storage/blobs.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>

#include <atomic>
#include <list>
#include <mutex>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  namespace {
    class SessionTestCredential final : public Azure::Core::Credentials::TokenCredential {
    public:
      SessionTestCredential() : TokenCredential("SessionTestCredential") {}

      mutable int GetTokenCount = 0;

      Azure::Core::Credentials::AccessToken GetToken(
          const Azure::Core::Credentials::TokenRequestContext&,
          const Azure::Core::Context&) const override
      {
        ++GetTokenCount;
        return {"bearer-token", Azure::DateTime::clock::now() + std::chrono::hours(1)};
      }
    };

    class CountingTokenCredential final : public Azure::Core::Credentials::TokenCredential {
    public:
      explicit CountingTokenCredential(
          std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential)
          : TokenCredential("CountingTokenCredential"), m_credential(std::move(credential))
      {
      }

      Azure::Core::Credentials::AccessToken GetToken(
          const Azure::Core::Credentials::TokenRequestContext& tokenRequestContext,
          const Azure::Core::Context& context) const override
      {
        ++m_getTokenCount;
        return m_credential->GetToken(tokenRequestContext, context);
      }

      int GetTokenCount() const { return m_getTokenCount.load(); }

    private:
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> m_credential;
      mutable std::atomic<int> m_getTokenCount{0};
    };

    struct SessionTestState final
    {
      bool RejectFirstSessionRequest = false;
      bool RejectedSessionRequest = false;
      bool RejectCreateSession = false;
      bool RejectCreateSessionWithNotFound = false;
      bool RejectDownloadWithNotFound = false;
      bool FailFirstDownloadWithServerError = false;
      bool FailedDownloadWithServerError = false;
      std::vector<std::string> CreateSessionContainers;
      std::vector<std::string> CreateSessionVersions;
      std::vector<std::string> AuthorizationHeaders;
      std::vector<std::string> DownloadHosts;
      std::vector<bool> SessionSignaturesValid;
      bool MutatedDownloadObserved = false;
      std::list<std::vector<uint8_t>> ResponseBodies;
    };

    class SessionTestTransport final : public Azure::Core::Http::HttpTransport {
    public:
      explicit SessionTestTransport(std::shared_ptr<SessionTestState> state)
          : m_state(std::move(state))
      {
      }

      std::unique_ptr<Azure::Core::Http::RawResponse> Send(
          Azure::Core::Http::Request& request,
          Azure::Core::Context const&) override
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto authorization = request.GetHeader("Authorization");
        m_state->AuthorizationHeaders.emplace_back(
            authorization.HasValue() ? authorization.Value() : std::string());

        const auto query = request.GetUrl().GetQueryParameters();
        if (request.GetMethod() == Azure::Core::Http::HttpMethod::Post && query.count("comp") != 0
            && query.at("comp") == "session")
        {
          m_state->CreateSessionContainers.emplace_back(request.GetUrl().GetPath());
          const auto version = request.GetHeader("x-ms-version");
          m_state->CreateSessionVersions.emplace_back(
              version.HasValue() ? version.Value() : std::string());
          if (m_state->RejectCreateSession)
          {
            return CreateResponse(Azure::Core::Http::HttpStatusCode::Forbidden, "Forbidden");
          }
          if (m_state->RejectCreateSessionWithNotFound)
          {
            return CreateResponse(Azure::Core::Http::HttpStatusCode::NotFound, "Not Found");
          }
          const auto token = "token-" + request.GetUrl().GetPath();
          const auto key = Azure::Core::Convert::Base64Encode(
              std::vector<uint8_t>{'s', 'e', 's', 's', 'i', 'o', 'n'});
          const std::string body
              = "<CreateSessionResult><Id>id</Id>"
                "<Expiration>2099-01-01T00:00:00Z</Expiration>"
                "<AuthenticationType>HMAC</AuthenticationType><Credentials><SessionToken>"
              + token + "</SessionToken><SessionKey>" + key
              + "</SessionKey></Credentials></CreateSessionResult>";
          return CreateResponse(
              Azure::Core::Http::HttpStatusCode::Created,
              "Created",
              std::vector<uint8_t>(body.begin(), body.end()));
        }

        if (authorization.HasValue() && authorization.Value().find("Session ") == 0)
        {
          const auto key = Azure::Core::Convert::Base64Encode(
              std::vector<uint8_t>{'s', 'e', 's', 's', 'i', 'o', 'n'});
          const auto signatureSeparator = authorization.Value().find(':');
          const auto expectedAuthorization = authorization.Value().substr(0, signatureSeparator + 1)
              + Azure::Storage::_internal::SharedKeyPolicy::GetSignature(request, "account", key);
          m_state->SessionSignaturesValid.emplace_back(
              signatureSeparator != std::string::npos
              && authorization.Value() == expectedAuthorization);
        }
        if (query.count("session-test") != 0 && request.GetHeader("x-ms-session-test").HasValue())
        {
          m_state->MutatedDownloadObserved = true;
        }
        m_state->DownloadHosts.emplace_back(request.GetUrl().GetHost());

        if (m_state->RejectFirstSessionRequest && !m_state->RejectedSessionRequest
            && authorization.HasValue() && authorization.Value().find("Session ") == 0)
        {
          m_state->RejectedSessionRequest = true;
          auto response
              = CreateResponse(Azure::Core::Http::HttpStatusCode::Unauthorized, "Unauthorized");
          response->SetHeader("WWW-Authenticate", "Session error=session_expired");
          return response;
        }

        if (m_state->FailFirstDownloadWithServerError && !m_state->FailedDownloadWithServerError)
        {
          m_state->FailedDownloadWithServerError = true;
          return CreateResponse(
              Azure::Core::Http::HttpStatusCode::InternalServerError, "Internal Server Error");
        }

        if (m_state->RejectDownloadWithNotFound)
        {
          return CreateResponse(Azure::Core::Http::HttpStatusCode::NotFound, "Not Found");
        }

        auto response = CreateResponse(Azure::Core::Http::HttpStatusCode::Ok, "OK");
        response->SetHeader("Content-Length", "0");
        response->SetHeader("x-ms-creation-time", "Mon, 01 Jan 2024 00:00:00 GMT");
        response->SetHeader("x-ms-server-encrypted", "true");
        return response;
      }

    private:
      std::unique_ptr<Azure::Core::Http::RawResponse> CreateResponse(
          Azure::Core::Http::HttpStatusCode statusCode,
          const std::string& reasonPhrase,
          std::vector<uint8_t> body = {}) const
      {
        m_state->ResponseBodies.emplace_back(std::move(body));
        auto response
            = std::make_unique<Azure::Core::Http::RawResponse>(1, 1, statusCode, reasonPhrase);
        response->SetBodyStream(
            std::make_unique<Azure::Core::IO::MemoryBodyStream>(m_state->ResponseBodies.back()));
        return response;
      }

      std::shared_ptr<SessionTestState> m_state;
      mutable std::mutex m_mutex;
    };

    class SessionRequestMutationPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
    public:
      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<SessionRequestMutationPolicy>();
      }

      std::unique_ptr<Azure::Core::Http::RawResponse> Send(
          Azure::Core::Http::Request& request,
          Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
          Azure::Core::Context const& context) const override
      {
        request.GetUrl().AppendQueryParameter("session-test", "value");
        request.SetHeader("x-ms-session-test", "value");
        return nextPolicy.Send(request, context);
      }
    };

    class DataLocalityTestPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
    public:
      DataLocalityTestPolicy() : m_downloadCount(std::make_shared<std::atomic<int>>(0)) {}

      explicit DataLocalityTestPolicy(std::shared_ptr<std::atomic<int>> downloadCount)
          : m_downloadCount(std::move(downloadCount))
      {
      }

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<DataLocalityTestPolicy>(m_downloadCount);
      }

      std::unique_ptr<Azure::Core::Http::RawResponse> Send(
          Azure::Core::Http::Request& request,
          Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
          Azure::Core::Context const& context) const override
      {
        const auto query = request.GetUrl().GetQueryParameters();
        if (request.GetMethod() == Azure::Core::Http::HttpMethod::Get && query.count("comp") == 0)
        {
          request.SetHeader("Host", request.GetUrl().GetHost());
          request.GetUrl().SetHost(
              ++*m_downloadCount == 1 ? "tenant1.blob.core.windows.net"
                                      : "tenant2.blob.core.windows.net");
        }
        return nextPolicy.Send(request, context);
      }

    private:
      std::shared_ptr<std::atomic<int>> m_downloadCount;
    };

    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> CreateSessionRequestCountingPolicy(
        std::shared_ptr<std::atomic<int>> sessionRequestCount)
    {
      return std::make_unique<PeekHttpRequestPolicy>(
          [sessionRequestCount
           = std::move(sessionRequestCount)](const Azure::Core::Http::Request& request) {
            const auto query = request.GetUrl().GetQueryParameters();
            if (request.GetMethod() == Azure::Core::Http::HttpMethod::Post
                && query.count("comp") != 0 && query.at("comp") == "session")
            {
              ++*sessionRequestCount;
            }
          });
    }

    void AddSessionTestTransport(
        Blobs::BlobClientOptions& options,
        const std::shared_ptr<SessionTestState>& state)
    {
      options.Transport.Transport = std::make_shared<SessionTestTransport>(state);
    }

    void EnableSessions(
        Blobs::BlobClientOptions& options,
        const std::shared_ptr<SessionTestState>& state)
    {
      AddSessionTestTransport(options, state);
      options.Session.Mode = Blobs::SessionMode::Enabled;
    }
  } // namespace

  TEST(SessionAuthenticationTest, SignsEligibleDownload)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    EnableSessions(options, state);
    options.Session.AccountName = "account";

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->AuthorizationHeaders.size(), 2U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session token-container:"), 0U);
    ASSERT_EQ(state->SessionSignaturesValid.size(), 1U);
    EXPECT_TRUE(state->SessionSignaturesValid[0]);
    EXPECT_EQ(credential->GetTokenCount, 1);
  }

  TEST(SessionAuthenticationTest, SignsAfterUserPerRetryPolicies)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    EnableSessions(options, state);
    options.PerRetryPolicies.emplace_back(std::make_unique<SessionRequestMutationPolicy>());
    options.Session.AccountName = "account";

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    EXPECT_TRUE(state->MutatedDownloadObserved);
    ASSERT_EQ(state->SessionSignaturesValid.size(), 1U);
    EXPECT_TRUE(state->SessionSignaturesValid[0]);
  }

  TEST(SessionAuthenticationTest, FallsBackToBearerAfter401)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    state->RejectFirstSessionRequest = true;
    Blobs::BlobClientOptions options;
    EnableSessions(options, state);
    options.Session.AccountName = "account";

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->AuthorizationHeaders.size(), 3U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session token-container:"), 0U);
    EXPECT_EQ(state->AuthorizationHeaders[2].find("Session "), std::string::npos);
    EXPECT_EQ(credential->GetTokenCount, 2);
  }

  TEST(SessionAuthenticationTest, UsesBearerOnSecondaryHost)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    state->FailFirstDownloadWithServerError = true;
    Blobs::BlobClientOptions options;
    options.Retry.MaxRetries = 1;
    options.Retry.RetryDelay = std::chrono::milliseconds(0);
    options.Retry.MaxRetryDelay = std::chrono::milliseconds(0);
    options.SecondaryHostForRetryReads = "account-secondary.blob.core.windows.net";
    EnableSessions(options, state);
    options.Session.AccountName = "account";

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    ASSERT_EQ(state->DownloadHosts.size(), 3U);
    EXPECT_EQ(state->DownloadHosts[0], "account.blob.core.windows.net");
    EXPECT_EQ(state->DownloadHosts[1], "account-secondary.blob.core.windows.net");
    EXPECT_EQ(state->DownloadHosts[2], "account.blob.core.windows.net");
    ASSERT_EQ(state->AuthorizationHeaders.size(), 4U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session "), 0U);
    EXPECT_EQ(state->AuthorizationHeaders[2].find("Bearer "), 0U);
    EXPECT_EQ(state->AuthorizationHeaders[3].find("Session "), 0U);
    EXPECT_EQ(credential->GetTokenCount, 2);
  }

  TEST(SessionAuthenticationTest, ReusesSessionAcrossDataLocalityTenants)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    options.PerOperationPolicies.emplace_back(std::make_unique<DataLocalityTestPolicy>());
    EnableSessions(options, state);
    options.Session.AccountName = "account";

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    ASSERT_EQ(state->DownloadHosts.size(), 2U);
    EXPECT_EQ(state->DownloadHosts[0], "tenant1.blob.core.windows.net");
    EXPECT_EQ(state->DownloadHosts[1], "tenant2.blob.core.windows.net");
    ASSERT_EQ(state->AuthorizationHeaders.size(), 3U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session "), 0U);
    EXPECT_EQ(state->AuthorizationHeaders[2].find("Session "), 0U);
    EXPECT_EQ(credential->GetTokenCount, 1);
  }

  TEST(SessionAuthenticationTest, SharedProviderCachesPerContainer)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions providerOptions;
    EnableSessions(providerOptions, state);
    providerOptions.Session.AccountName = "account";
    auto provider = std::make_shared<Blobs::ContainerSessionProvider>(
        "https://account.blob.core.windows.net", credential, providerOptions);

    auto clientOptions = providerOptions;
    clientOptions.Session.Provider = provider;
    Blobs::BlobServiceClient serviceClient(
        "https://account.blob.core.windows.net", credential, clientOptions);
    auto first = serviceClient.GetBlobContainerClient("container1").GetBlobClient("blob1");
    auto second = serviceClient.GetBlobContainerClient("container1").GetBlobClient("blob2");
    auto third = serviceClient.GetBlobContainerClient("container2").GetBlobClient("blob");

    EXPECT_NO_THROW(first.Download());
    EXPECT_NO_THROW(second.Download());
    EXPECT_NO_THROW(third.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 2U);
    EXPECT_EQ(state->CreateSessionContainers[0], "container1");
    EXPECT_EQ(state->CreateSessionContainers[1], "container2");
    EXPECT_EQ(state->CreateSessionVersions[0], Blobs::_detail::ApiVersion);
    EXPECT_EQ(state->CreateSessionVersions[1], Blobs::_detail::ApiVersion);
  }

  TEST(SessionAuthenticationTest, SharedProviderSupportsIndependentClients)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions providerOptions;
    EnableSessions(providerOptions, state);
    providerOptions.Session.AccountName = "account";
    auto provider = std::make_shared<Blobs::ContainerSessionProvider>(
        "https://account.blob.core.windows.net", credential, providerOptions);

    auto clientOptions = providerOptions;
    clientOptions.Session.Provider = provider;
    Blobs::BlobClient first(
        "https://account.blob.core.windows.net/container/blob1", credential, clientOptions);
    Blobs::BlobClient second(
        "https://account.blob.core.windows.net/container/blob2", credential, clientOptions);

    EXPECT_NO_THROW(first.Download());
    EXPECT_NO_THROW(second.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    EXPECT_EQ(state->CreateSessionContainers[0], "container");
    ASSERT_EQ(state->AuthorizationHeaders.size(), 3U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session "), 0U);
    EXPECT_EQ(state->AuthorizationHeaders[2].find("Session "), 0U);
  }

  TEST(SessionAuthenticationTest, ExplicitProviderSupportsCustomEndpoint)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    EnableSessions(options, state);
    options.Session.AccountName = "account";
    options.Session.Provider = std::make_shared<Blobs::ContainerSessionProvider>(
        "https://storage.contoso.com", credential, options);

    Blobs::BlobClient client("https://storage.contoso.com/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    EXPECT_EQ(state->CreateSessionContainers[0], "container");
    ASSERT_EQ(state->AuthorizationHeaders.size(), 2U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session token-container:"), 0U);
  }

  TEST(SessionAuthenticationTest, ExplicitProviderSupportsPathStyleEndpoint)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    EnableSessions(options, state);
    options.Session.AccountName = "account";
    options.Session.Provider = std::make_shared<Blobs::ContainerSessionProvider>(
        "https://localhost/account", credential, options);

    Blobs::BlobClient client("https://localhost/account/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    EXPECT_EQ(state->CreateSessionContainers[0], "account/container");
    ASSERT_EQ(state->AuthorizationHeaders.size(), 2U);
    EXPECT_EQ(state->AuthorizationHeaders[1].find("Session token-account/container:"), 0U);
  }

  TEST(SessionAuthenticationTest, CustomEndpointRequiresAccountNameAndProvider)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    Blobs::BlobClientOptions options;
    options.Session.Mode = Blobs::SessionMode::Enabled;
    options.Session.AccountName = "account";

    EXPECT_THROW(
        Blobs::BlobClient("https://storage.contoso.com/container/blob", credential, options),
        std::invalid_argument);
  }

  TEST(SessionAuthenticationTest, AutoModeUsesBearer)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    AddSessionTestTransport(options, state);

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    EXPECT_TRUE(state->CreateSessionContainers.empty());
    ASSERT_EQ(state->AuthorizationHeaders.size(), 1U);
    EXPECT_EQ(state->AuthorizationHeaders[0].find("Bearer "), 0U);
    EXPECT_EQ(credential->GetTokenCount, 1);
  }

  TEST(SessionAuthenticationTest, AutoModeAllowsCustomEndpointConstruction)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    Blobs::BlobClientOptions options;

    EXPECT_NO_THROW(
        Blobs::BlobClient("https://storage.contoso.com/container/blob", credential, options));
  }

  TEST(SessionAuthenticationTest, EnabledModeRejectsCustomEndpointWithoutConfiguration)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    Blobs::BlobClientOptions options;
    options.Session.Mode = Blobs::SessionMode::Enabled;

    EXPECT_THROW(
        Blobs::BlobClient("https://storage.contoso.com/container/blob", credential, options),
        std::invalid_argument);
  }

  TEST(SessionAuthenticationTest, DefaultProviderCachesSoftFailure)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    state->RejectCreateSession = true;
    Blobs::BlobClientOptions options;
    options.Retry.MaxRetries = 0;
    EnableSessions(options, state);
    options.Session.AccountName = "account";

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());
    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
  }

  TEST(SessionAuthenticationTest, PropagatesCreateSessionNotFound)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    state->RejectCreateSessionWithNotFound = true;
    Blobs::BlobClientOptions options;
    options.Retry.MaxRetries = 0;
    EnableSessions(options, state);

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/missing-container/blob", credential, options);

    EXPECT_THROW(client.Download(), StorageException);
    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    EXPECT_EQ(state->CreateSessionContainers[0], "missing-container");
  }

  TEST(SessionAuthenticationTest, PropagatesDownloadNotFound)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    state->RejectDownloadWithNotFound = true;
    Blobs::BlobClientOptions options;
    options.Retry.MaxRetries = 0;
    EnableSessions(options, state);

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/missing-blob", credential, options);

    EXPECT_THROW(client.Download(), StorageException);
    ASSERT_EQ(state->CreateSessionContainers.size(), 1U);
    EXPECT_EQ(state->CreateSessionContainers[0], "container");
  }

  TEST(SessionAuthenticationTest, DisabledModeUsesBearer)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    auto state = std::make_shared<SessionTestState>();
    Blobs::BlobClientOptions options;
    AddSessionTestTransport(options, state);
    options.Session.Mode = Blobs::SessionMode::Disabled;

    Blobs::BlobClient client(
        "https://account.blob.core.windows.net/container/blob", credential, options);
    EXPECT_NO_THROW(client.Download());

    EXPECT_TRUE(state->CreateSessionContainers.empty());
    ASSERT_EQ(state->AuthorizationHeaders.size(), 1U);
    EXPECT_EQ(state->AuthorizationHeaders[0].find("Bearer "), 0U);
    EXPECT_EQ(credential->GetTokenCount, 1);
  }

  TEST(SessionAuthenticationTest, DisabledModeAllowsCustomEndpointConstruction)
  {
    auto credential = std::make_shared<SessionTestCredential>();
    Blobs::BlobClientOptions options;
    options.Session.Mode = Blobs::SessionMode::Disabled;

    EXPECT_NO_THROW(
        Blobs::BlobClient("https://storage.contoso.com/container/blob", credential, options));
  }

  class SessionAuthenticationRecordedTest : public BlobContainerClientTest {
  };

  TEST_F(SessionAuthenticationRecordedTest, DownloadsWithDefaultSessionProvider_PLAYBACKONLY_)
  {
    auto blobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());
    const std::vector<uint8_t> content{'s', 'e', 's', 's', 'i', 'o', 'n'};
    blobClient.UploadFrom(content.data(), content.size());

    auto credential = std::make_shared<CountingTokenCredential>(GetTestCredential());
    auto options = InitStorageClientOptions<Blobs::BlobClientOptions>();
    options.Session.Mode = Blobs::SessionMode::Enabled;
    auto sessionRequestCount = std::make_shared<std::atomic<int>>(0);
    options.PerRetryPolicies.emplace_back(CreateSessionRequestCountingPolicy(sessionRequestCount));
    Blobs::BlobClient sessionClient(blobClient.GetUrl(), credential, options);

    for (int i = 0; i < 3; ++i)
    {
      EXPECT_NO_THROW(sessionClient.Download().Value.BodyStream->ReadToEnd());
    }
    EXPECT_EQ(credential->GetTokenCount(), 1);
    EXPECT_EQ(sessionRequestCount->load(), 1);

    Blobs::Models::BlobHttpHeaders headers;
    headers.ContentType = "application/octet-stream";
    EXPECT_NO_THROW(sessionClient.GetProperties());
    EXPECT_NO_THROW(sessionClient.SetMetadata({{"key", "value"}}));
    EXPECT_NO_THROW(sessionClient.SetHttpHeaders(headers));
    EXPECT_EQ(credential->GetTokenCount(), 2);
    EXPECT_EQ(sessionRequestCount->load(), 1);
  }

  TEST_F(
      SessionAuthenticationRecordedTest,
      IndependentClientsShareExplicitSessionProvider_PLAYBACKONLY_)
  {
    auto blobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());
    const std::vector<uint8_t> content{'s', 'e', 's', 's', 'i', 'o', 'n'};
    blobClient.UploadFrom(content.data(), content.size());

    auto credential = std::make_shared<CountingTokenCredential>(GetTestCredential());
    auto options = InitStorageClientOptions<Blobs::BlobClientOptions>();
    options.Session.Mode = Blobs::SessionMode::Enabled;
    auto sessionRequestCount = std::make_shared<std::atomic<int>>(0);
    options.PerRetryPolicies.emplace_back(CreateSessionRequestCountingPolicy(sessionRequestCount));
    options.Session.Provider = std::make_shared<Blobs::ContainerSessionProvider>(
        GetBlobServiceUrl(), credential, options);
    options.Session.AccountName = m_accountName;
    Blobs::BlobClient firstClient(blobClient.GetUrl(), credential, options);
    Blobs::BlobClient secondClient(blobClient.GetUrl(), credential, options);

    EXPECT_NO_THROW(firstClient.Download().Value.BodyStream->ReadToEnd());
    EXPECT_NO_THROW(secondClient.Download().Value.BodyStream->ReadToEnd());
    EXPECT_EQ(credential->GetTokenCount(), 1);
    EXPECT_EQ(sessionRequestCount->load(), 1);

    Blobs::Models::BlobHttpHeaders headers;
    headers.ContentType = "application/octet-stream";
    EXPECT_NO_THROW(firstClient.GetProperties());
    EXPECT_NO_THROW(firstClient.SetMetadata({{"key", "value"}}));
    EXPECT_NO_THROW(firstClient.SetHttpHeaders(headers));
    EXPECT_EQ(credential->GetTokenCount(), 2);
    EXPECT_EQ(sessionRequestCount->load(), 1);
  }

}}} // namespace Azure::Storage::Test
