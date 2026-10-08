// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "datalake_file_client_test.hpp"

#include <azure/storage/files/datalake.hpp>

#include <atomic>
#include <list>
#include <mutex>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  namespace {
    struct SessionRequest final
    {
      Azure::Core::Http::HttpMethod Method;
      std::string Host;
      std::string Authorization;
      std::string Component;
    };

    struct SessionTestState final
    {
      std::vector<SessionRequest> Requests;
      std::list<std::vector<uint8_t>> ResponseBodies;
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

    class SessionTestCredential final : public Azure::Core::Credentials::TokenCredential {
    public:
      SessionTestCredential() : TokenCredential("SessionTestCredential") {}

      Azure::Core::Credentials::AccessToken GetToken(
          const Azure::Core::Credentials::TokenRequestContext&,
          const Azure::Core::Context&) const override
      {
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
        const auto query = request.GetUrl().GetQueryParameters();
        const auto component = query.find("comp");
        m_state->Requests.push_back(
            {request.GetMethod(),
             request.GetUrl().GetHost(),
             authorization.HasValue() ? authorization.Value() : std::string(),
             component == query.end() ? std::string() : component->second});

        if (request.GetMethod() == Azure::Core::Http::HttpMethod::Post && component != query.end()
            && component->second == "session")
        {
          const auto key = Azure::Core::Convert::Base64Encode(
              std::vector<uint8_t>{'s', 'e', 's', 's', 'i', 'o', 'n'});
          const std::string body = "<CreateSessionResult><Id>id</Id>"
                                   "<Expiration>2099-01-01T00:00:00Z</Expiration>"
                                   "<AuthenticationType>HMAC</AuthenticationType><Credentials>"
                                   "<SessionToken>session-token</SessionToken><SessionKey>"
              + key + "</SessionKey></Credentials></CreateSessionResult>";
          return CreateResponse(
              Azure::Core::Http::HttpStatusCode::Created,
              "Created",
              std::vector<uint8_t>(body.begin(), body.end()));
        }

        if (request.GetMethod() == Azure::Core::Http::HttpMethod::Put)
        {
          auto response = CreateResponse(Azure::Core::Http::HttpStatusCode::Created, "Created");
          response->SetHeader("ETag", "\"etag\"");
          response->SetHeader("Last-Modified", "Mon, 01 Jan 2024 00:00:00 GMT");
          response->SetHeader("x-ms-request-server-encrypted", "true");
          return response;
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
          std::vector<uint8_t> body = {})
      {
        m_state->ResponseBodies.emplace_back(std::move(body));
        auto response
            = std::make_unique<Azure::Core::Http::RawResponse>(1, 1, statusCode, reasonPhrase);
        response->SetBodyStream(
            std::make_unique<Azure::Core::IO::MemoryBodyStream>(m_state->ResponseBodies.back()));
        return response;
      }

      std::shared_ptr<SessionTestState> m_state;
      std::mutex m_mutex;
    };

    Files::DataLake::DataLakeClientOptions CreateSessionOptions(
        const std::shared_ptr<SessionTestState>& state)
    {
      Files::DataLake::DataLakeClientOptions options;
      options.Transport.Transport = std::make_shared<SessionTestTransport>(state);
      options.Session.Mode = Blobs::SessionMode::Enabled;
      options.Session.AccountName = "account";
      return options;
    }
  } // namespace

  TEST(DataLakeSessionAuthenticationTest, DownloadUsesBlobSessionPipeline)
  {
    auto state = std::make_shared<SessionTestState>();
    Files::DataLake::DataLakeFileClient client(
        "https://account.dfs.core.windows.net/filesystem/file",
        std::make_shared<SessionTestCredential>(),
        CreateSessionOptions(state));

    EXPECT_NO_THROW(client.Download());

    ASSERT_EQ(state->Requests.size(), 2U);
    EXPECT_EQ(state->Requests[0].Method, Azure::Core::Http::HttpMethod::Post);
    EXPECT_EQ(state->Requests[0].Host, "account.blob.core.windows.net");
    EXPECT_EQ(state->Requests[0].Component, "session");
    EXPECT_EQ(state->Requests[0].Authorization.find("Bearer "), 0U);
    EXPECT_EQ(state->Requests[1].Method, Azure::Core::Http::HttpMethod::Get);
    EXPECT_EQ(state->Requests[1].Host, "account.blob.core.windows.net");
    EXPECT_EQ(state->Requests[1].Authorization.find("Session "), 0U);
  }

  TEST(DataLakeSessionAuthenticationTest, DfsOperationsUseBearer)
  {
    auto state = std::make_shared<SessionTestState>();
    Files::DataLake::DataLakeFileClient client(
        "https://account.dfs.core.windows.net/filesystem/file",
        std::make_shared<SessionTestCredential>(),
        CreateSessionOptions(state));

    EXPECT_NO_THROW(client.Create());

    ASSERT_EQ(state->Requests.size(), 1U);
    EXPECT_EQ(state->Requests[0].Method, Azure::Core::Http::HttpMethod::Put);
    EXPECT_EQ(state->Requests[0].Host, "account.dfs.core.windows.net");
    EXPECT_EQ(state->Requests[0].Authorization.find("Bearer "), 0U);
  }

  TEST(DataLakeSessionAuthenticationTest, RenamedClientRetainsBlobSessionPipeline)
  {
    auto state = std::make_shared<SessionTestState>();
    Files::DataLake::DataLakeFileSystemClient client(
        "https://account.dfs.core.windows.net/filesystem",
        std::make_shared<SessionTestCredential>(),
        CreateSessionOptions(state));

    auto renamedClient = client.RenameFile("source", "destination").Value;
    EXPECT_NO_THROW(renamedClient.Download());

    ASSERT_EQ(state->Requests.size(), 3U);
    EXPECT_EQ(state->Requests[0].Method, Azure::Core::Http::HttpMethod::Put);
    EXPECT_EQ(state->Requests[0].Host, "account.dfs.core.windows.net");
    EXPECT_EQ(state->Requests[0].Authorization.find("Bearer "), 0U);
    EXPECT_EQ(state->Requests[1].Method, Azure::Core::Http::HttpMethod::Post);
    EXPECT_EQ(state->Requests[1].Host, "account.blob.core.windows.net");
    EXPECT_EQ(state->Requests[1].Component, "session");
    EXPECT_EQ(state->Requests[2].Method, Azure::Core::Http::HttpMethod::Get);
    EXPECT_EQ(state->Requests[2].Host, "account.blob.core.windows.net");
    EXPECT_EQ(state->Requests[2].Authorization.find("Session "), 0U);
  }

  class DataLakeSessionAuthenticationRecordedTest : public DataLakeFileClientTest {
  };

  TEST_F(DataLakeSessionAuthenticationRecordedTest, DownloadUsesSessionAuthentication_PLAYBACKONLY_)
  {
    auto options = InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>();
    options.Session.Mode = Blobs::SessionMode::Enabled;
    auto sessionRequestCount = std::make_shared<std::atomic<int>>(0);
    options.PerRetryPolicies.emplace_back(CreateSessionRequestCountingPolicy(sessionRequestCount));
    auto credential = std::make_shared<CountingTokenCredential>(GetTestCredential());
    Files::DataLake::DataLakeFileClient client(m_fileClient->GetUrl(), credential, options);

    EXPECT_NO_THROW(client.Download().Value.Body->ReadToEnd());
    EXPECT_NO_THROW(client.Download().Value.Body->ReadToEnd());
    EXPECT_EQ(sessionRequestCount->load(), 1);
    EXPECT_EQ(credential->GetTokenCount(), 1);
    EXPECT_NO_THROW(client.GetProperties());
    EXPECT_EQ(sessionRequestCount->load(), 1);
    EXPECT_EQ(credential->GetTokenCount(), 2);
  }

}}} // namespace Azure::Storage::Test
