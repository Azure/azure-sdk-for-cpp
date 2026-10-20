// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/storage/common/internal/storage_pipeline.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  extern Core::Context::Key const BlobsProtocolHeadersKey;

  class BlobsProtocolPolicy final : public Core::Http::Policies::HttpPolicy {
  public:
    std::unique_ptr<Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<BlobsProtocolPolicy>();
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      auto const accept = request.GetHeader("Accept");
      if (accept.HasValue()
          && (accept.Value() == "application/xml" || accept.Value() == "application/json"
              || accept.Value() == "application/octet-stream"))
      {
        request.RemoveHeader("Accept");
      }
      auto const contentType = request.GetHeader("Content-Type");
      if (contentType.HasValue() && contentType.Value() == "application/octet-stream")
      {
        request.RemoveHeader("Content-Type");
      }
      else if (contentType.HasValue() && contentType.Value() == "application/xml")
      {
        request.SetHeader("Content-Type", "application/xml; charset=UTF-8");
      }

      Core::CaseInsensitiveMap additionalHeaders;
      if (context.TryGetValue(BlobsProtocolHeadersKey, additionalHeaders))
      {
        for (auto const& header : additionalHeaders)
        {
          request.SetHeader(header.first, header.second);
        }
      }
      return nextPolicy.Send(request, context);
    }
  };

  inline std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> BuildBlobsPipelinePolicies(
      Core::_internal::ClientOptions const& options,
      _internal::BuildStoragePipelineOptions pipelineOptions)
  {
    auto policies = _internal::BuildHttpPipelinePolicies(options, std::move(pipelineOptions));
    policies.insert(policies.begin(), std::make_unique<BlobsProtocolPolicy>());
    return policies;
  }

}}}} // namespace Azure::Storage::Blobs::_detail
