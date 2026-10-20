// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/storage/common/internal/storage_pipeline.hpp>

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace _detail {

  class SharesProtocolPolicy final : public Core::Http::Policies::HttpPolicy {
  public:
    std::unique_ptr<Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<SharesProtocolPolicy>();
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      request.RemoveHeader("Accept");
      auto const contentType = request.GetHeader("Content-Type");
      if (contentType.HasValue() && contentType.Value() == "application/octet-stream")
      {
        request.RemoveHeader("Content-Type");
      }
      else if (contentType.HasValue() && contentType.Value() == "application/xml")
      {
        request.SetHeader("Content-Type", "application/xml; charset=UTF-8");
      }
      return nextPolicy.Send(request, context);
    }
  };

  inline std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> BuildSharesPipelinePolicies(
      Core::_internal::ClientOptions const& options,
      _internal::BuildStoragePipelineOptions pipelineOptions)
  {
    auto policies = _internal::BuildHttpPipelinePolicies(options, std::move(pipelineOptions));
    policies.insert(policies.begin(), std::make_unique<SharesProtocolPolicy>());
    return policies;
  }

}}}}} // namespace Azure::Storage::Files::Shares::_detail
