// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_batch_client.hpp"

#include <algorithm>
#include <cstring>
#include <memory>

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/blobs/version.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  namespace {
    class NoopTransportPolicy : public Core::Http::HttpPolicy {
    public:
      ~NoopTransportPolicy() override {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<NoopTransportPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Context const& context,
          Core::Http::Request& request,
          Core::Http::NextHttpPolicy nextHttpPolicy) const override
      {
        unused(context, request, nextHttpPolicy);
        return std::unique_ptr<Core::Http::RawResponse>();
      }
    };
  } // namespace

  int32_t BlobBatch::DeleteBlob(
      const std::string& containerName,
      const std::string& blobName,
      const DeleteBlobOptions& options)
  {
    DeleteBlobSubRequest operation;
    operation.ContainerName = containerName;
    operation.BlobName = blobName;
    operation.Options = options;
    m_deleteBlobSubRequests.emplace_back(std::move(operation));
    return static_cast<int32_t>(m_deleteBlobSubRequests.size() - 1);
  }

  int32_t BlobBatch::SetBlobAccessTier(
      const std::string& containerName,
      const std::string& blobName,
      AccessTier tier,
      const SetBlobAccessTierOptions& options)
  {
    SetBlobAccessTierSubRequest operation;
    operation.ContainerName = containerName;
    operation.BlobName = blobName;
    operation.Options = options;
    operation.Tier = tier;
    m_setBlobAccessTierSubRequests.emplace_back(std::move(operation));
    return static_cast<int32_t>(m_setBlobAccessTierSubRequests.size() - 1);
  }

  BlobBatchClient BlobBatchClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobBatchClientOptions& options)
  {
    auto parsedConnectionString = Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.BlobServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobBatchClient(
          serviceUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobBatchClient(serviceUri.GetAbsoluteUrl(), options);
    }
  }

  BlobBatchClient::BlobBatchClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobBatchClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Storage::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);

    policies.clear();
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<NoopTransportPolicy>());
    m_subRequestPipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobBatchClient::BlobBatchClient(
      const std::string& serviceUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobBatchClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Storage::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);

    policies.clear();
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Details::c_StorageScope));
    policies.emplace_back(std::make_unique<NoopTransportPolicy>());
    m_subRequestPipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobBatchClient::BlobBatchClient(
      const std::string& serviceUri,
      const BlobBatchClientOptions& options)
      : m_serviceUrl(serviceUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Azure::Storage::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);

    policies.clear();
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<NoopTransportPolicy>());
    m_subRequestPipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  Azure::Core::Response<SubmitBlobBatchResult> BlobBatchClient::SubmitBatch(
      const BlobBatch& batch,
      const SubmitBlobBatchOptions& options) const
  {
    const std::string c_lineEnding = "\r\n";
    const std::string c_contentTypePrefix = "multipart/mixed; boundary=";

    std::string boundary = "batch_" + Azure::Core::Uuid::CreateUuid().GetUuidString();

    enum class RequestType
    {
      DeleteBlob,
      SetBlobAccessTier,
    };

    std::vector<RequestType> requestTypes;

    std::string requestBody;
    {
      auto getBatchBoundary = [&c_lineEnding, &boundary, subRequestCounter = 0]() mutable {
        std::string ret;
        ret += "--" + boundary + c_lineEnding;
        ret += "Content-Type: application/http" + c_lineEnding + "Content-Transfer-Encoding: binary"
            + c_lineEnding + "Content-ID: " + std::to_string(subRequestCounter++) + c_lineEnding
            + c_lineEnding;
        return ret;
      };
      for (const auto& subrequest : batch.m_deleteBlobSubRequests)
      {
        requestTypes.emplace_back(RequestType::DeleteBlob);

        requestBody += getBatchBoundary();

        auto blobUrl = m_serviceUrl;
        blobUrl.AppendPath(Details::UrlEncodePath(subrequest.ContainerName));
        blobUrl.AppendPath(Details::UrlEncodePath(subrequest.BlobName));
        BlobRestClient::Blob::DeleteBlobOptions protocolLayerOptions;
        protocolLayerOptions.DeleteSnapshots = subrequest.Options.DeleteSnapshots;
        protocolLayerOptions.IfModifiedSince = subrequest.Options.AccessConditions.IfModifiedSince;
        protocolLayerOptions.IfUnmodifiedSince
            = subrequest.Options.AccessConditions.IfUnmodifiedSince;
        protocolLayerOptions.IfMatch = subrequest.Options.AccessConditions.IfMatch;
        protocolLayerOptions.IfNoneMatch = subrequest.Options.AccessConditions.IfNoneMatch;
        protocolLayerOptions.LeaseId = subrequest.Options.AccessConditions.LeaseId;
        auto message = BlobRestClient::Blob::DeleteCreateMessage(blobUrl, protocolLayerOptions);
        message.RemoveHeader(Details::c_HttpHeaderXMsVersion);
        m_subRequestPipeline->Send(options.Context, message);
        requestBody += message.GetHTTPMessagePreBody();
      }
      for (const auto& subrequest : batch.m_setBlobAccessTierSubRequests)
      {
        requestTypes.emplace_back(RequestType::SetBlobAccessTier);

        requestBody += getBatchBoundary();

        auto blobUrl = m_serviceUrl;
        blobUrl.AppendPath(Details::UrlEncodePath(subrequest.ContainerName));
        blobUrl.AppendPath(Details::UrlEncodePath(subrequest.BlobName));
        BlobRestClient::Blob::SetBlobAccessTierOptions protocolLayerOptions;
        protocolLayerOptions.Tier = subrequest.Tier;
        protocolLayerOptions.RehydratePriority = subrequest.Options.RehydratePriority;
        auto message
            = BlobRestClient::Blob::SetAccessTierCreateMessage(blobUrl, protocolLayerOptions);
        message.RemoveHeader(Details::c_HttpHeaderXMsVersion);
        m_subRequestPipeline->Send(options.Context, message);
        requestBody += message.GetHTTPMessagePreBody();
      }
      requestBody += "--" + boundary + "--" + c_lineEnding;
    }

    BlobRestClient::BlobBatch::SubmitBlobBatchOptions protocolLayerOptions;
    protocolLayerOptions.ContentType = c_contentTypePrefix + boundary;

    Azure::Core::Http::MemoryBodyStream requestBodyStream(
        reinterpret_cast<const uint8_t*>(requestBody.data()), requestBody.length());

    auto rawResponse = BlobRestClient::BlobBatch::SubmitBatch(
        options.Context, *m_pipeline, m_serviceUrl, &requestBodyStream, protocolLayerOptions);

    if (rawResponse->ContentType.substr(0, c_contentTypePrefix.length()) == c_contentTypePrefix)
    {
      boundary = rawResponse->ContentType.substr(c_contentTypePrefix.length());
    }
    else
    {
      throw std::runtime_error("failed to parse Content-Type response header");
    }

    SubmitBlobBatchResult batchResult;
    {
      const std::vector<uint8_t>& responseBody = rawResponse.GetRawResponse().GetBody();

      const char* const startPos = reinterpret_cast<const char*>(responseBody.data());
      const char* currPos = startPos;
      const char* const endPos = currPos + responseBody.size();

      auto parseLookAhead = [&currPos, endPos](const std::string& expect) -> bool {
        // This doesn't move currPos
        for (std::size_t i = 0; i < expect.length(); ++i)
        {
          if (currPos + i < endPos && currPos[i] == expect[i])
          {
            continue;
          }
          return false;
        }
        return true;
      };

      auto parseConsume = [&currPos, startPos, &parseLookAhead](const std::string& expect) -> void {
        // This moves currPos
        if (parseLookAhead(expect))
        {
          currPos += expect.length();
        }
        else
        {
          throw std::runtime_error(
              "failed to parse response body at " + std::to_string(currPos - startPos));
        }
      };

      auto parseFindNext = [&currPos, endPos](const std::string& expect) -> const char* {
        // This doesn't move currPos
        return std::search(currPos, endPos, expect.begin(), expect.end());
      };

      auto parseFindNextAfter = [endPos, &parseFindNext](const std::string& expect) -> const char* {
        // This doesn't move currPos
        return std::min(endPos, parseFindNext(expect) + expect.length());
      };

      auto parseGetUntilAfter
          = [&currPos, endPos, &parseFindNext](const std::string& expect) -> std::string {
        // This moves currPos
        auto ePos = parseFindNext(expect);
        std::string ret(currPos, ePos);
        currPos = std::min(endPos, ePos + expect.length());
        return ret;
      };

      int subRequestCounter = 0;
      while (true)
      {
        parseConsume("--" + boundary);

        if (parseLookAhead("--"))
        {
          parseConsume("--");
        }

        if (currPos == endPos)
        {
          break;
        }

        currPos = parseFindNextAfter(c_lineEnding + c_lineEnding);
        auto boundaryPos = parseFindNext("--" + boundary);

        // now (currPos, boundaryPos) is a subresponse body
        parseConsume("HTTP/");
        int32_t httpMajorVersion = std::stoi(parseGetUntilAfter("."));
        int32_t httpMinorVersion = std::stoi(parseGetUntilAfter(" "));
        int32_t httpStatusCode = std::stoi(parseGetUntilAfter(" "));
        std::string httpReasonPhrase = parseGetUntilAfter(c_lineEnding);

        auto rawSubresponse = std::make_unique<Azure::Core::Http::RawResponse>(
            httpMajorVersion,
            httpMinorVersion,
            static_cast<Azure::Core::Http::HttpStatusCode>(httpStatusCode),
            httpReasonPhrase);

        while (currPos < boundaryPos)
        {
          if (parseLookAhead(c_lineEnding))
          {
            break;
          }

          std::string headerName = parseGetUntilAfter(": ");
          std::string headerValue = parseGetUntilAfter(c_lineEnding);
          rawSubresponse->AddHeader(headerName, headerValue);
        }

        parseConsume(c_lineEnding);

        rawSubresponse->SetBody(std::vector<uint8_t>(currPos, boundaryPos));
        currPos = boundaryPos;

        RequestType requestType = requestTypes[subRequestCounter++];
        if (requestType == RequestType::DeleteBlob)
        {
          try
          {
            batchResult.DeleteBlobResults.emplace_back(BlobRestClient::Blob::DeleteCreateResponse(
                options.Context, std::move(rawSubresponse)));
          }
          catch (StorageError& e)
          {
            batchResult.DeleteBlobResults.emplace_back(Azure::Core::Response<DeleteBlobResult>(
                DeleteBlobResult{}, std::move(e.RawResponse)));
          }
        }
        else if (requestType == RequestType::SetBlobAccessTier)
        {
          try
          {
            batchResult.SetBlobAccessTierResults.emplace_back(
                BlobRestClient::Blob::SetAccessTierCreateResponse(
                    options.Context, std::move(rawSubresponse)));
          }
          catch (StorageError& e)
          {
            batchResult.SetBlobAccessTierResults.emplace_back(
                Azure::Core::Response<SetBlobAccessTierResult>(
                    SetBlobAccessTierResult{}, std::move(e.RawResponse)));
          }
        }
      }
    }

    return Azure::Core::Response<SubmitBlobBatchResult>(
        std::move(batchResult), rawResponse.ExtractRawResponse());
  }
}}} // namespace Azure::Storage::Blobs
