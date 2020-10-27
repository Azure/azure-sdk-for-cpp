// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs.hpp"
#include "test_base.hpp"

#include <chrono>
#include <memory>
#include <vector>

namespace Azure { namespace Storage { namespace Test {

  class MockTransportPolicy : public Core::Http::HttpPolicy {
  public:
    MockTransportPolicy() {}

    explicit MockTransportPolicy(std::string primaryContent)
        : m_primaryContent(std::make_shared<std::string>(std::move(primaryContent))),
          m_primaryETag(c_dummyETag)
    {
    }

    explicit MockTransportPolicy(std::string primaryContent, std::string secondaryContent)
        : m_primaryContent(std::make_shared<std::string>(std::move(primaryContent))),
          m_secondaryContent(std::make_shared<std::string>(std::move(secondaryContent))),
          m_primaryETag(c_dummyETag),
          m_secondaryETag(*m_secondaryContent == *m_primaryContent ? c_dummyETag : c_dummyETag2)
    {
    }

    ~MockTransportPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<MockTransportPolicy>(*this);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Context const& context,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override
    {
      unused(context, nextHttpPolicy);

      const auto requestHeaders = request.GetHeaders();
      int64_t requestOffset = 0;
      int64_t requestLength = std::numeric_limits<int64_t>::max();
      {
        std::string rangeStr;
        if (requestHeaders.find("Range") != requestHeaders.end())
        {
          rangeStr = requestHeaders.at("Range");
        }
        else if (requestHeaders.find("x-ms-range") != requestHeaders.end())
        {
          rangeStr = requestHeaders.at("x-ms-range");
        }
        if (!rangeStr.empty())
        {
          auto equalPos = rangeStr.find('=');
          auto dashPos = rangeStr.find('-');
          std::string startByte = rangeStr.substr(equalPos + 1, dashPos - equalPos - 1);
          requestOffset = std::stoll(startByte);
          std::string endByte = rangeStr.substr(dashPos + 1);
          if (!endByte.empty())
          {
            requestLength = std::stoll(endByte) - requestOffset + 1;
          }
        }
      }

      auto ConstructTransportException
          = []() { return Azure::Core::Http::TransportException("Error while sending request. "); };
      auto ConstructNotFoundResponse = []() {
        auto requestId = Core::Uuid::CreateUuid().GetUuidString();
        std::string errorResponseBody
            = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
              "<Error><Code>BlobNotFound</Code><Message>The specified blob does not exist.\n"
              "RequestId:"
            + requestId + "\nTime:2020-09-11T02:09:31.8962056Z</Message></Error>";
        auto response = std::make_unique<Core::Http::RawResponse>(Core::Http::RawResponse(
            1, 1, Core::Http::HttpStatusCode::NotFound, "The specified blob does not exist."));
        response->SetBody(std::vector<uint8_t>(errorResponseBody.begin(), errorResponseBody.end()));
        response->AddHeader("content-length", std::to_string(errorResponseBody.length()));
        response->AddHeader("content-type", "application/xml");
        response->AddHeader("x-ms-request-id", Core::Uuid::CreateUuid().GetUuidString());
        response->AddHeader("x-ms-version", Blobs::c_ApiVersion);
        response->AddHeader("x-ms-error-code", "BlobNotFound");
        response->AddHeader("date", ToRfc1123(std::chrono::system_clock::now()));
        return response;
      };
      auto ConstructPreconditionFailedResponse = []() {
        auto requestId = Core::Uuid::CreateUuid().GetUuidString();
        std::string errorResponseBody
            = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
              "<Error><Code>ConditionNotMet</Code>"
              "<Message>The condition specified using HTTP conditional header(s) is not met.\n"
              "RequestId:"
            + requestId + "\nTime:2020-09-11T02:01:26.0151739Z</Message></Error>";
        auto response = std::make_unique<Core::Http::RawResponse>(Core::Http::RawResponse(
            1,
            1,
            Core::Http::HttpStatusCode::PreconditionFailed,
            "The condition specified using HTTP conditional header(s) is not met."));
        response->SetBody(std::vector<uint8_t>(errorResponseBody.begin(), errorResponseBody.end()));
        response->AddHeader("content-length", std::to_string(errorResponseBody.length()));
        response->AddHeader("content-type", "application/xml");
        response->AddHeader("x-ms-request-id", Core::Uuid::CreateUuid().GetUuidString());
        response->AddHeader("x-ms-version", Blobs::c_ApiVersion);
        response->AddHeader("x-ms-error-code", "ConditionNotMet");
        response->AddHeader("date", ToRfc1123(std::chrono::system_clock::now()));
        return response;
      };
      auto ConstructPrimaryResponse
          = [requestOffset, requestLength, this, ConstructNotFoundResponse]() {
              if (!m_primaryContent)
              {
                return ConstructNotFoundResponse();
              }
              auto response = std::make_unique<Core::Http::RawResponse>(
                  Core::Http::RawResponse(1, 1, Core::Http::HttpStatusCode::Ok, "OK"));
              int64_t bodyLength = std::min(
                  static_cast<int64_t>(m_primaryContent->length()) - requestOffset, requestLength);
              auto bodyStream = std::make_unique<Core::Http::MemoryBodyStream>(
                  reinterpret_cast<const uint8_t*>(m_primaryContent->data() + requestOffset),
                  bodyLength);
              response->SetBodyStream(std::move(bodyStream));
              response->AddHeader("content-length", std::to_string(bodyLength));
              response->AddHeader("etag", m_primaryETag);
              response->AddHeader("last-modified", "Thu 27 Aug 2001 07:00:00 GMT");
              response->AddHeader("x-ms-request-id", Core::Uuid::CreateUuid().GetUuidString());
              response->AddHeader("x-ms-version", Blobs::c_ApiVersion);
              response->AddHeader("x-ms-creation-time", "Thu 27 Aug 2002 07:00:00 GMT");
              response->AddHeader("x-ms-lease-status", "unlocked");
              response->AddHeader("x-ms-lease-state", "available");
              response->AddHeader("x-ms-blob-type", "BlockBlob");
              response->AddHeader("x-ms-server-encrypted", "true");
              response->AddHeader("date", ToRfc1123(std::chrono::system_clock::now()));
              return response;
            };
      auto ConstructSecondaryResponse =
          [requestOffset, requestLength, this, ConstructNotFoundResponse]() {
            if (!m_secondaryContent)
            {
              return ConstructNotFoundResponse();
            }
            auto response = std::make_unique<Core::Http::RawResponse>(
                Core::Http::RawResponse(1, 1, Core::Http::HttpStatusCode::Ok, "OK"));
            int64_t bodyLength = std::min(
                static_cast<int64_t>(m_secondaryContent->length()) - requestOffset, requestLength);
            auto bodyStream = std::make_unique<Core::Http::MemoryBodyStream>(
                reinterpret_cast<const uint8_t*>(m_secondaryContent->data() + requestOffset),
                bodyLength);
            response->SetBodyStream(std::move(bodyStream));
            response->AddHeader("content-length", std::to_string(bodyLength));
            response->AddHeader("etag", m_secondaryETag);
            response->AddHeader("last-modified", "Thu 27 Aug 2001 07:00:00 GMT");
            response->AddHeader("x-ms-request-id", Core::Uuid::CreateUuid().GetUuidString());
            response->AddHeader("x-ms-version", Blobs::c_ApiVersion);
            response->AddHeader("x-ms-creation-time", "Thu 27 Aug 2002 07:00:00 GMT");
            response->AddHeader("x-ms-lease-status", "unlocked");
            response->AddHeader("x-ms-lease-state", "available");
            response->AddHeader("x-ms-blob-type", "BlockBlob");
            response->AddHeader("x-ms-server-encrypted", "true");
            response->AddHeader("date", ToRfc1123(std::chrono::system_clock::now()));
            return response;
          };

      Region region = Region::Primary;
      if (request.GetUrl().GetHost().find("-secondary") != std::string::npos)
      {
        region = Region::Secondary;
      }

      if (m_failPolicy)
      {
        auto responseType = m_failPolicy(region);
        switch (responseType)
        {
          case ResponseType::NotFound:
            return ConstructNotFoundResponse();
          case ResponseType::PreconditionFailed:
            return ConstructPreconditionFailedResponse();
          case ResponseType::TransportException:
            throw ConstructTransportException();
          default:;
        }
      }

      if (region == Region::Primary)
      {
        if (requestHeaders.find("if-match") == requestHeaders.end()
            || requestHeaders.at("if-match") == m_primaryETag)
        {
          return ConstructPrimaryResponse();
        }
        return ConstructPreconditionFailedResponse();
      }
      else
      {
        if (requestHeaders.find("if-match") == requestHeaders.end()
            || requestHeaders.at("if-match") == m_secondaryETag)
        {
          return ConstructSecondaryResponse();
        }
        return ConstructPreconditionFailedResponse();
      }
    }

    enum Region
    {
      Primary,
      Secondary,
    };

    enum ResponseType
    {
      Success,
      NotFound,
      PreconditionFailed,
      TransportException,
    };

    void SetFailPolicy(std::function<ResponseType(Region)> func) { m_failPolicy = std::move(func); }

  private:
    std::shared_ptr<std::string> m_primaryContent;
    std::shared_ptr<std::string> m_secondaryContent;
    std::string m_primaryETag;
    std::string m_secondaryETag;

    std::function<ResponseType(Region)> m_failPolicy;
  };

  TEST(StorageRetryPolicyTest, Basic)
  {
    std::string primaryContent = "primary content";
    auto transportPolicyPtr = std::make_unique<MockTransportPolicy>(primaryContent);
    Blobs::BlobClientOptions clientOptions;
    clientOptions.PerRetryPolicies.emplace_back(std::move(transportPolicyPtr));
    auto blobClient = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), RandomString(), RandomString(), clientOptions);
    auto ret = blobClient.Download();
    auto responseBody
        = Azure::Core::Http::BodyStream::ReadToEnd(Azure::Core::Context(), *(ret->BodyStream));
    EXPECT_EQ(std::string(responseBody.begin(), responseBody.end()), primaryContent);
  }

  TEST(StorageRetryPolicyTest, Retry)
  {
    std::string primaryContent = "primary content";
    auto transportPolicyPtr = std::make_unique<MockTransportPolicy>(primaryContent);

    int numTrial = 0;
    auto failPolicy
        = [&numTrial](MockTransportPolicy::Region region) -> MockTransportPolicy::ResponseType {
      unused(region);
      if (numTrial++ == 0)
        return MockTransportPolicy::ResponseType::TransportException;
      return MockTransportPolicy::ResponseType::Success;
    };

    transportPolicyPtr->SetFailPolicy(failPolicy);

    Blobs::BlobClientOptions clientOptions;
    clientOptions.PerRetryPolicies.emplace_back(std::move(transportPolicyPtr));
    int64_t delayMs = 1000;
    clientOptions.RetryOptions.RetryDelay = std::chrono::milliseconds(delayMs);
    auto blobClient = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), RandomString(), RandomString(), clientOptions);
    auto timeBegin = std::chrono::steady_clock::now();
    auto ret = blobClient.Download();
    auto timeEnd = std::chrono::steady_clock::now();
    auto responseBody
        = Azure::Core::Http::BodyStream::ReadToEnd(Azure::Core::Context(), *(ret->BodyStream));
    EXPECT_EQ(std::string(responseBody.begin(), responseBody.end()), primaryContent);
    EXPECT_EQ(numTrial, 2);

    int64_t elapsedTime
        = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count();
    EXPECT_GE(elapsedTime, delayMs * 0.5);
    EXPECT_LE(elapsedTime, delayMs * 2);
  }

  TEST(StorageRetryPolicyTest, Failover)
  {
    std::string primaryContent = "primary content";
    std::string secondaryContent = "secondary content";
    auto transportPolicyPtr
        = std::make_unique<MockTransportPolicy>(primaryContent, secondaryContent);

    auto failPolicy = [](MockTransportPolicy::Region region) -> MockTransportPolicy::ResponseType {
      if (region == MockTransportPolicy::Region::Primary)
      {
        return MockTransportPolicy::ResponseType::TransportException;
      }
      return MockTransportPolicy::ResponseType::Success;
    };

    transportPolicyPtr->SetFailPolicy(failPolicy);

    Blobs::BlobClientOptions clientOptions;
    clientOptions.PerRetryPolicies.emplace_back(std::move(transportPolicyPtr));
    clientOptions.RetryOptions.RetryDelay = std::chrono::milliseconds(0);
    {
      std::string primaryUrl
          = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
                StandardStorageConnectionString(), RandomString(), RandomString())
                .GetUri();
      std::string secondaryUrl = InferSecondaryUri(primaryUrl);
      std::string secondaryHost = Core::Http::Url(secondaryUrl).GetHost();
      clientOptions.RetryOptions.SecondaryHostForRetryReads = secondaryHost;
    }
    auto blobClient = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), RandomString(), RandomString(), clientOptions);
    auto ret = blobClient.Download();
    auto responseBody
        = Azure::Core::Http::BodyStream::ReadToEnd(Azure::Core::Context(), *(ret->BodyStream));
    EXPECT_EQ(std::string(responseBody.begin(), responseBody.end()), secondaryContent);
  }

  TEST(StorageRetryPolicyTest, Secondary404)
  {
    std::string primaryContent = "primary content";
    std::string secondaryContent = "secondary content";
    auto transportPolicyPtr
        = std::make_unique<MockTransportPolicy>(primaryContent, secondaryContent);

    int numPrimaryTrial = 0;
    int numSecondaryTrial = 0;
    auto failPolicy = [&numPrimaryTrial, &numSecondaryTrial](
                          MockTransportPolicy::Region region) -> MockTransportPolicy::ResponseType {
      if (region == MockTransportPolicy::Region::Primary)
      {
        if (numPrimaryTrial++ < 2)
        {
          return MockTransportPolicy::ResponseType::TransportException;
        }
        return MockTransportPolicy::ResponseType::Success;
      }
      else
      {
        numSecondaryTrial++;
        return MockTransportPolicy::ResponseType::NotFound;
      }
    };

    transportPolicyPtr->SetFailPolicy(failPolicy);

    Blobs::BlobClientOptions clientOptions;
    clientOptions.PerRetryPolicies.emplace_back(std::move(transportPolicyPtr));
    clientOptions.RetryOptions.MaxRetries = 3;
    clientOptions.RetryOptions.RetryDelay = std::chrono::milliseconds(0);
    {
      std::string primaryUrl
          = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
                StandardStorageConnectionString(), RandomString(), RandomString())
                .GetUri();
      std::string secondaryUrl = InferSecondaryUri(primaryUrl);
      std::string secondaryHost = Core::Http::Url(secondaryUrl).GetHost();
      clientOptions.RetryOptions.SecondaryHostForRetryReads = secondaryHost;
    }
    auto blobClient = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), RandomString(), RandomString(), clientOptions);
    auto ret = blobClient.Download();
    auto responseBody
        = Azure::Core::Http::BodyStream::ReadToEnd(Azure::Core::Context(), *(ret->BodyStream));
    EXPECT_EQ(std::string(responseBody.begin(), responseBody.end()), primaryContent);
    EXPECT_EQ(numPrimaryTrial, 3);
    EXPECT_EQ(numSecondaryTrial, 1);
  }

  TEST(StorageRetryPolicyTest, Secondary412)
  {
    std::string primaryContent = "primary content";
    std::string secondaryContent = "secondary content";
    auto transportPolicyPtr
        = std::make_unique<MockTransportPolicy>(primaryContent, secondaryContent);

    int numPrimaryTrial = 0;
    int numSecondaryTrial = 0;
    auto failPolicy = [&numPrimaryTrial, &numSecondaryTrial](
                          MockTransportPolicy::Region region) -> MockTransportPolicy::ResponseType {
      if (region == MockTransportPolicy::Region::Primary)
      {
        numPrimaryTrial++;
        if (numPrimaryTrial % 2 == 1)
        {
          return MockTransportPolicy::ResponseType::Success;
        }
        else
        {
          return MockTransportPolicy::ResponseType::TransportException;
        }
      }
      else
      {
        numSecondaryTrial++;
        return MockTransportPolicy::ResponseType::Success;
      }
    };

    transportPolicyPtr->SetFailPolicy(failPolicy);

    Blobs::BlobClientOptions clientOptions;
    clientOptions.PerRetryPolicies.emplace_back(std::move(transportPolicyPtr));
    clientOptions.RetryOptions.MaxRetries = 3;
    clientOptions.RetryOptions.RetryDelay = std::chrono::milliseconds(0);
    {
      std::string primaryUrl
          = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
                StandardStorageConnectionString(), RandomString(), RandomString())
                .GetUri();
      std::string secondaryUrl = InferSecondaryUri(primaryUrl);
      std::string secondaryHost = Core::Http::Url(secondaryUrl).GetHost();
      clientOptions.RetryOptions.SecondaryHostForRetryReads = secondaryHost;
    }
    auto blobClient = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), RandomString(), RandomString(), clientOptions);
    std::string downloadBuffer;
    downloadBuffer.resize(std::max(primaryContent.size(), secondaryContent.size()));
    Blobs::DownloadBlobToOptions options;
    options.InitialChunkSize = 2;
    options.ChunkSize = 2;
    options.Concurrency = 1;
    blobClient.DownloadTo(
        reinterpret_cast<uint8_t*>(&downloadBuffer[0]),
        static_cast<int64_t>(downloadBuffer.size()),
        options);

    downloadBuffer.resize(primaryContent.size());
    EXPECT_EQ(downloadBuffer, primaryContent);
    EXPECT_NE(numPrimaryTrial, 0);
    EXPECT_NE(numSecondaryTrial, 0);
  }

}}} // namespace Azure::Storage::Test
