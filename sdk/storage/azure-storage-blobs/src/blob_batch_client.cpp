// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_batch_client.hpp"

#include <algorithm>
#include <cstring>
#include <functional>
#include <future>
#include <type_traits>
#include <vector>

#include <azure/core/azure_assert.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  namespace _detail {
    DeferredResponseSharedBase::~DeferredResponseSharedBase() {}
  } // namespace _detail

  namespace {
    const std::string LineEnding = "\r\n";
    const std::string BatchContentTypePrefix = "multipart/mixed; boundary=";
    static Core::Context::Key s_batchKey;
    static Core::Context::Key s_subrequestKey;
    static Core::Context::Key s_subresponseKey;

    struct Parser final
    {
      explicit Parser(const std::string& str)
          : startPos(str.data()), currPos(startPos), endPos(startPos + str.length())
      {
      }
      explicit Parser(const std::vector<uint8_t>& str)
          : startPos(reinterpret_cast<const char*>(str.data())),
            currPos(reinterpret_cast<const char*>(startPos)),
            endPos(reinterpret_cast<const char*>(startPos) + str.size())
      {
      }
      const char* startPos;
      const char* currPos;
      const char* endPos;

      bool IsEnd() const { return currPos == endPos; }

      bool LookAhead(const std::string& expect) const
      {
        for (size_t i = 0; i < expect.length(); ++i)
        {
          if (currPos + i < endPos && currPos[i] == expect[i])
          {
            continue;
          }
          return false;
        }
        return true;
      }

      void Consume(const std::string& expect)
      {
        // This moves currPos
        if (LookAhead(expect))
        {
          currPos += expect.length();
        }
        else
        {
          throw std::runtime_error(
              "failed to parse response body at " + std::to_string(currPos - startPos));
        }
      }

      const char* FindNext(const std::string& expect) const
      {
        return std::search(currPos, endPos, expect.begin(), expect.end());
      }

      const char* AfterNext(const std::string& expect) const
      {
        return std::min(endPos, FindNext(expect) + expect.length());
      }

      std::string GetBeforeNextAndConsume(const std::string& expect)
      {
        // This moves currPos
        auto ePos = FindNext(expect);
        std::string ret(currPos, ePos);
        currPos = std::min(endPos, ePos + expect.length());
        return ret;
      }
    };

    std::unique_ptr<Core::Http::RawResponse> ParseRawResponse(const std::string& responseText)
    {
      Parser parser(responseText);

      parser.Consume("HTTP/");
      int32_t httpMajorVersion = std::stoi(parser.GetBeforeNextAndConsume("."));
      int32_t httpMinorVersion = std::stoi(parser.GetBeforeNextAndConsume(" "));
      int32_t httpStatusCode = std::stoi(parser.GetBeforeNextAndConsume(" "));
      const std::string httpReasonPhrase = parser.GetBeforeNextAndConsume(LineEnding);

      auto rawResponse = std::make_unique<Azure::Core::Http::RawResponse>(
          httpMajorVersion,
          httpMinorVersion,
          static_cast<Azure::Core::Http::HttpStatusCode>(httpStatusCode),
          httpReasonPhrase);

      while (!parser.IsEnd())
      {
        if (parser.LookAhead(LineEnding))
        {
          break;
        }
        std::string headerName = parser.GetBeforeNextAndConsume(": ");
        std::string headerValue = parser.GetBeforeNextAndConsume(LineEnding);
        rawResponse->SetHeader(headerName, headerValue);
      }
      parser.Consume(LineEnding);
      rawResponse->SetBody(std::vector<uint8_t>(parser.currPos, parser.endPos));

      return rawResponse;
    }

    class StringBodyStream final : public Core::IO::BodyStream {
    public:
      explicit StringBodyStream(std::string content) : m_content(std::move(content)) {}
      StringBodyStream(const StringBodyStream&) = delete;
      StringBodyStream& operator=(const StringBodyStream&) = delete;
      StringBodyStream(StringBodyStream&& other) = default;
      StringBodyStream& operator=(StringBodyStream&& other) = default;
      ~StringBodyStream() override {}
      int64_t Length() const override { return m_content.length(); }
      void Rewind() override { m_offset = 0; }

    private:
      size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override
      {
        (void)context;
        size_t copy_length = std::min(count, m_content.length() - m_offset);
        std::memcpy(buffer, &m_content[0] + m_offset, static_cast<size_t>(copy_length));
        m_offset += copy_length;
        return copy_length;
      }

    private:
      std::string m_content;
      size_t m_offset = 0;
    };

    class RemoveXMsVersionPolicy final : public Core::Http::Policies::HttpPolicy {
    public:
      ~RemoveXMsVersionPolicy() override {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<RemoveXMsVersionPolicy>(*this);
      }
      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Http::Policies::NextHttpPolicy nextPolicy,
          const Core::Context& context) const override
      {
        request.RemoveHeader(_internal::HttpHeaderXMsVersion);
        return nextPolicy.Send(request, context);
      }
    };

    class NoopTransportPolicy final : public Core::Http::Policies::HttpPolicy {
    public:
      ~NoopTransportPolicy() override {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<NoopTransportPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Http::Policies::NextHttpPolicy nextPolicy,
          const Core::Context& context) const override
      {
        (void)nextPolicy;

        std::string* subrequestText = nullptr;
        context.TryGetValue(s_subrequestKey, subrequestText);

        if (subrequestText)
        {
          std::string requestText = request.GetMethod().ToString() + " /"
              + request.GetUrl().GetRelativeUrl() + " HTTP/1.1" + LineEnding;
          for (const auto& header : request.GetHeaders())
          {
            requestText += header.first + ": " + header.second + LineEnding;
          }
          requestText += LineEnding;
          *subrequestText = std::move(requestText);

          auto rawResponse = std::make_unique<Core::Http::RawResponse>(
              1, 1, Core::Http::HttpStatusCode::Accepted, "Accepted");
          return rawResponse;
        }

        std::string* subresponseText = nullptr;
        context.TryGetValue(s_subresponseKey, subresponseText);
        if (subresponseText)
        {
          return ParseRawResponse(*subresponseText);
        }
        AZURE_UNREACHABLE_CODE();
      }
    };

    class ConstructBatchRequestBodyPolicy final : public Core::Http::Policies::HttpPolicy {
    public:
      ConstructBatchRequestBodyPolicy(
          std::function<void(Core::Http::Request&, const Core::Context&)> constructRequestFunction,
          std::function<void(std::unique_ptr<Core::Http::RawResponse>&, const Core::Context&)>
              parseResponseFunction)
          : m_constructRequestFunction(std::move(constructRequestFunction)),
            m_parseResponseFunction(std::move(parseResponseFunction))
      {
      }
      ~ConstructBatchRequestBodyPolicy() override {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<ConstructBatchRequestBodyPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Http::Policies::NextHttpPolicy nextPolicy,
          const Core::Context& context) const override
      {
        m_constructRequestFunction(request, context);
        auto rawResponse = nextPolicy.Send(request, context);
        m_parseResponseFunction(rawResponse, context);
        return rawResponse;
      }

    private:
      std::function<void(Core::Http::Request&, const Core::Context&)> m_constructRequestFunction;
      std::function<void(std::unique_ptr<Core::Http::RawResponse>&, const Core::Context&)>
          m_parseResponseFunction;
    };

    struct DeferredDeleteBlobShared
        : public _detail::DeferredResponseShared<Models::DeleteBlobResult>
    {
      ~DeferredDeleteBlobShared() override {}

      Response<Models::DeleteBlobResult> GetResponse() override
      {
        return Promise.get_future().get().Value();
      }

      std::string BlobUrl;
      Nullable<Blobs::BlobClient> BlobClient;
      DeleteBlobOptions Options;
      // Need to use Nullable here to get around a bug in Visual Studio. Ref
      // https://developercommunity.visualstudio.com/t/c-shared-state-futuresstate-default-constructs-the/60897
      std::promise<Nullable<Response<Models::DeleteBlobResult>>> Promise;
    };

    struct DeferredSetBlobAccessTierShared
        : public _detail::DeferredResponseShared<Models::SetBlobAccessTierResult>
    {
      ~DeferredSetBlobAccessTierShared() override {}

      Response<Models::SetBlobAccessTierResult> GetResponse() override
      {
        return Promise.get_future().get().Value();
      }

      std::string BlobUrl;
      Nullable<Blobs::BlobClient> BlobClient;
      Models::AccessTier Tier;
      SetBlobAccessTierOptions Options;
      std::promise<Nullable<Response<Models::SetBlobAccessTierResult>>> Promise;
    };
  } // namespace

  DeferredResponse<Models::DeleteBlobResult> BlobBatch::DeleteBlob(
      const std::string& blobContainerName,
      const std::string& blobName,
      const DeleteBlobOptions& options)
  {
    auto deferredResponse = std::make_shared<DeferredDeleteBlobShared>();
    auto blobUrl = m_url;
    blobUrl.AppendPath(blobContainerName);
    blobUrl.AppendPath(blobName);
    deferredResponse->BlobUrl = blobUrl.GetAbsoluteUrl();
    deferredResponse->Options = options;
    return CreateDeferredResponse<Models::DeleteBlobResult>(deferredResponse);
  }

  DeferredResponse<Models::DeleteBlobResult> BlobBatch::DeleteBlob(
      const std::string& blobUrl,
      const DeleteBlobOptions& options)
  {
    auto deferredResponse = std::make_shared<DeferredDeleteBlobShared>();
    deferredResponse->BlobUrl = blobUrl;
    deferredResponse->Options = options;
    return CreateDeferredResponse<Models::DeleteBlobResult>(deferredResponse);
  }

  DeferredResponse<Models::SetBlobAccessTierResult> BlobBatch::SetBlobAccessTier(
      const std::string& blobContainerName,
      const std::string& blobName,
      Models::AccessTier tier,
      const SetBlobAccessTierOptions& options)
  {
    auto deferredResponse = std::make_shared<DeferredSetBlobAccessTierShared>();
    auto blobUrl = m_url;
    blobUrl.AppendPath(blobContainerName);
    blobUrl.AppendPath(blobName);
    deferredResponse->BlobUrl = blobUrl.GetAbsoluteUrl();
    deferredResponse->Tier = tier;
    deferredResponse->Options = options;
    return CreateDeferredResponse<Models::SetBlobAccessTierResult>(deferredResponse);
  }

  DeferredResponse<Models::SetBlobAccessTierResult> BlobBatch::SetBlobAccessTier(
      const std::string& blobUrl,
      Models::AccessTier tier,
      const SetBlobAccessTierOptions& options)
  {
    auto deferredResponse = std::make_shared<DeferredSetBlobAccessTierShared>();
    deferredResponse->BlobUrl = blobUrl;
    deferredResponse->Tier = tier;
    deferredResponse->Options = options;
    return CreateDeferredResponse<Models::SetBlobAccessTierResult>(deferredResponse);
  }

  template <class T> void BlobBatchClient::Init(const T& blobServiceOrContainerClient)
  {
    struct PolicyPeeper final
    {
      std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> m_policies;
    };
    static_assert(sizeof(PolicyPeeper) == sizeof(Core::Http::_internal::HttpPipeline), "");

    std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> parentRequestPolicies;
    std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> subrequestPolicies;

    auto removeXMsVersionPolicy = std::make_unique<RemoveXMsVersionPolicy>();
    for (const auto& policyPtr :
         reinterpret_cast<const PolicyPeeper*>(blobServiceOrContainerClient.m_pipeline.get())
             ->m_policies)
    {
      auto policyRawPtr = policyPtr.get();
      parentRequestPolicies.push_back(policyPtr->Clone());
      if (dynamic_cast<Core::Http::Policies::_internal::RetryPolicy*>(policyRawPtr))
      {
        parentRequestPolicies.push_back(std::make_unique<ConstructBatchRequestBodyPolicy>(
            [this](Core::Http::Request& request, const Core::Context& context) {
              ConstructSubrequests(request, context);
            },
            [this](
                std::unique_ptr<Core::Http::RawResponse>& rawResponse,
                const Core::Context& context) { ParseSubresponses(rawResponse, context); }));
      }

      if (dynamic_cast<_internal::SharedKeyPolicy*>(policyRawPtr))
      {
        subrequestPolicies.push_back(std::move(removeXMsVersionPolicy));
        removeXMsVersionPolicy.reset();
      }

      if (!(dynamic_cast<Core::Http::Policies::_internal::RetryPolicy*>(policyRawPtr)
            || dynamic_cast<Core::Http::Policies::_internal::LogPolicy*>(policyRawPtr)
            || dynamic_cast<Core::Http::Policies::_internal::TransportPolicy*>(policyRawPtr)))
      {
        subrequestPolicies.push_back(policyPtr->Clone());
      }
    }
    if (removeXMsVersionPolicy)
    {
      subrequestPolicies.push_back(std::move(removeXMsVersionPolicy));
    }
    subrequestPolicies.push_back(std::make_unique<NoopTransportPolicy>());

    auto parentRequestPipeline
        = std::make_shared<Core::Http::_internal::HttpPipeline>(std::move(parentRequestPolicies));
    if (std::is_same<BlobServiceClient, T>::value)
    {
      m_blobServiceClient
          = reinterpret_cast<const BlobServiceClient&>(blobServiceOrContainerClient);
      m_blobServiceClient->m_pipeline = std::move(parentRequestPipeline);
    }
    else if (std::is_same<BlobContainerClient, T>::value)
    {
      m_blobContainerClient
          = reinterpret_cast<const BlobContainerClient&>(blobServiceOrContainerClient);
      m_blobContainerClient->m_pipeline = std::move(parentRequestPipeline);
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }

    m_subrequestPipeline
        = std::make_shared<Core::Http::_internal::HttpPipeline>(std::move(subrequestPolicies));
  }

  BlobBatchClient::BlobBatchClient(BlobServiceClient blobServiceClient) { Init(blobServiceClient); }

  BlobBatchClient::BlobBatchClient(BlobContainerClient blobContainerClient)
  {
    Init(blobContainerClient);
  }

  BlobBatch BlobBatchClient::CreateBatch()
  {
    if (m_blobServiceClient)
    {
      return BlobBatch(m_blobServiceClient->m_serviceUrl);
    }
    else
    {
      auto url = m_blobContainerClient->m_blobContainerUrl;
      auto path = url.GetPath();
      if (!path.empty() && path.back() == '/')
      {
        path.pop_back();
      }
      auto slash_pos = path.rfind('/');
      path = path.substr(0, slash_pos == std::string::npos ? 0 : (slash_pos + 1));
      url.SetPath(path);
      return BlobBatch(url);
    }
  }

  Azure::Response<Models::SubmitBlobBatchResult> BlobBatchClient::SubmitBatch(
      const BlobBatch& batch,
      const SubmitBlobBatchOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    if (m_blobServiceClient)
    {
      _detail::ServiceClient::SubmitServiceBatchOptions protocolLayerOptions;
      StringBodyStream bodyStream(std::string{});
      auto response = _detail::ServiceClient::SubmitBatch(
          *(m_blobServiceClient->m_pipeline),
          m_blobServiceClient->m_serviceUrl,
          bodyStream,
          protocolLayerOptions,
          context.WithValue(s_batchKey, &batch));
      return Azure::Response<Models::SubmitBlobBatchResult>(
          Models::SubmitBlobBatchResult(), std::move(response.RawResponse));
    }
    else
    {
      _detail::BlobContainerClient::SubmitBlobContainerBatchOptions protocolLayerOptions;
      StringBodyStream bodyStream(std::string{});
      auto response = _detail::BlobContainerClient::SubmitBatch(
          *(m_blobContainerClient->m_pipeline),
          m_blobContainerClient->m_blobContainerUrl,
          bodyStream,
          protocolLayerOptions,
          context.WithValue(s_batchKey, &batch));
      return Azure::Response<Models::SubmitBlobBatchResult>(
          Models::SubmitBlobBatchResult(), std::move(response.RawResponse));
    }
  }

  void BlobBatchClient::ConstructSubrequests(
      Core::Http::Request& request,
      const Core::Context& context)
  {
    const std::string boundary = "batch_" + Azure::Core::Uuid::CreateUuid().ToString();

    auto getBatchBoundary = [&boundary, subRequestCounter = 0]() mutable {
      std::string ret;
      ret += "--" + boundary + LineEnding;
      ret += "Content-Type: application/http" + LineEnding + "Content-Transfer-Encoding: binary"
          + LineEnding + "Content-ID: " + std::to_string(subRequestCounter++) + LineEnding
          + LineEnding;
      return ret;
    };

    auto getBlobClient = [this](const auto& subrequest) {
      auto blobClient = m_blobServiceClient
          ? m_blobServiceClient->GetBlobContainerClient("$").GetBlobClient("$")
          : m_blobContainerClient->GetBlobClient("$");
      blobClient.m_blobUrl = Core::Url(subrequest.BlobUrl);
      blobClient.m_pipeline = m_subrequestPipeline;
      return blobClient;
    };

    std::string requestBody;

    const BlobBatch* batch = nullptr;
    context.TryGetValue(s_batchKey, batch);

    for (const auto& subrequest : batch->DeferredOperations())
    {
      {
        auto subrequestPtr = std::dynamic_pointer_cast<DeferredDeleteBlobShared>(subrequest);
        if (subrequestPtr)
        {
          subrequestPtr->BlobClient = getBlobClient(*subrequestPtr);
          requestBody += getBatchBoundary();

          std::string subrequestText;
          subrequestPtr->BlobClient.Value().Delete(
              subrequestPtr->Options, Core::Context().WithValue(s_subrequestKey, &subrequestText));
          requestBody += subrequestText;
          continue;
        }
      }
      {
        auto subrequestPtr = std::dynamic_pointer_cast<DeferredSetBlobAccessTierShared>(subrequest);
        if (subrequestPtr)
        {
          subrequestPtr->BlobClient = getBlobClient(*subrequestPtr);
          requestBody += getBatchBoundary();

          std::string subrequestText;
          subrequestPtr->BlobClient.Value().SetAccessTier(
              subrequestPtr->Tier,
              subrequestPtr->Options,
              Core::Context().WithValue(s_subrequestKey, &subrequestText));
          requestBody += subrequestText;
          continue;
        }
      }
      AZURE_UNREACHABLE_CODE();
    }
    requestBody += "--" + boundary + "--" + LineEnding;

    request.SetHeader(_internal::HttpHeaderContentType, BatchContentTypePrefix + boundary);
    dynamic_cast<StringBodyStream&>(*request.GetBodyStream())
        = StringBodyStream(std::move(requestBody));
    request.SetHeader(
        _internal::HttpHeaderContentLength, std::to_string(request.GetBodyStream()->Length()));
  }

  void BlobBatchClient::ParseSubresponses(
      std::unique_ptr<Core::Http::RawResponse>& rawResponse,
      const Core::Context& context)
  {
    if (rawResponse->GetStatusCode() != Core::Http::HttpStatusCode::Accepted
        || rawResponse->GetHeaders().count(_internal::HttpHeaderContentType) == 0)
    {
      return;
    }

    const std::string boundary = rawResponse->GetHeaders()
                                     .at(std::string(_internal::HttpHeaderContentType))
                                     .substr(BatchContentTypePrefix.length());

    const std::vector<uint8_t>& responseBody = rawResponse->ExtractBodyStream()->ReadToEnd(context);
    Parser parser(responseBody);

    std::vector<std::string> subresponses;
    while (true)
    {
      parser.Consume("--" + boundary);
      if (parser.LookAhead("--"))
      {
        parser.Consume("--");
      }
      if (parser.IsEnd())
      {
        break;
      }
      auto contentIdPos = parser.AfterNext("Content-ID: ");
      auto responseStartPos = parser.AfterNext(LineEnding + LineEnding);
      auto responseEndPos = parser.FindNext("--" + boundary);
      if (contentIdPos != parser.endPos)
      {
        parser.currPos = contentIdPos;
        auto idEndPos = parser.FindNext(LineEnding);
        size_t id = static_cast<size_t>(std::stoi(std::string(parser.currPos, idEndPos)));
        if (subresponses.size() < id + 1)
        {
          subresponses.resize(id + 1);
        }
        subresponses[id] = std::string(responseStartPos, responseEndPos);
        parser.currPos = responseEndPos;
      }
      else
      {
        rawResponse = ParseRawResponse(std::string(responseStartPos, responseEndPos));
        parser.currPos = responseEndPos;
        return;
      }
    }

    const BlobBatch* batch = nullptr;
    context.TryGetValue(s_batchKey, batch);

    size_t subresponseCounter = 0;
    for (const auto& subrequest : batch->DeferredOperations())
    {
      {
        auto subrequestPtr = std::dynamic_pointer_cast<DeferredDeleteBlobShared>(subrequest);
        if (subrequestPtr)
        {
          try
          {
            auto response = subrequestPtr->BlobClient.Value().Delete(
                subrequestPtr->Options,
                Core::Context().WithValue(s_subresponseKey, &subresponses[subresponseCounter++]));
            subrequestPtr->Promise.set_value(std::move(response));
          }
          catch (...)
          {
            subrequestPtr->Promise.set_exception(std::current_exception());
          }
          continue;
        }
      }
      {
        auto subrequestPtr = std::dynamic_pointer_cast<DeferredSetBlobAccessTierShared>(subrequest);
        if (subrequestPtr)
        {
          try
          {
            auto response = subrequestPtr->BlobClient.Value().SetAccessTier(
                subrequestPtr->Tier,
                subrequestPtr->Options,
                Core::Context().WithValue(s_subresponseKey, &subresponses[subresponseCounter++]));
            subrequestPtr->Promise.set_value(std::move(response));
          }
          catch (...)
          {
            subrequestPtr->Promise.set_exception(std::current_exception());
          }
          continue;
        }
      }
      AZURE_UNREACHABLE_CODE();
    }
  }

}}} // namespace Azure::Storage::Blobs
