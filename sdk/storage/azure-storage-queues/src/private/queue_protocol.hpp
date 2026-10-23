// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/exception.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/internal/storage_pipeline.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <utility>

namespace Azure { namespace Storage { namespace Queues { namespace _detail {

  class QueueProtocolPolicy final : public Core::Http::Policies::HttpPolicy {
  public:
    std::unique_ptr<Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<QueueProtocolPolicy>();
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      // The previous protocol layer did not set Accept. User policies can still set it.
      request.RemoveHeader("Accept");
      auto const contentType = request.GetHeader("Content-Type");
      if (contentType.HasValue() && contentType.Value() == "application/xml")
      {
        if (request.GetBodyStream()->Length() == 0)
        {
          request.RemoveHeader("Content-Type");
        }
        else
        {
          request.SetHeader("Content-Type", "application/xml; charset=UTF-8");
        }
      }
      return nextPolicy.Send(request, context);
    }
  };

  inline std::vector<std::unique_ptr<Core::Http::Policies::HttpPolicy>> BuildQueuePipelinePolicies(
      Core::_internal::ClientOptions const& options,
      _internal::BuildStoragePipelineOptions pipelineOptions)
  {
    auto policies = _internal::BuildHttpPipelinePolicies(options, std::move(pipelineOptions));
    policies.insert(policies.begin(), std::make_unique<QueueProtocolPolicy>());
    return policies;
  }

  template <class Operation>
  auto InvokeStorageProtocol(Operation&& operation)
      -> decltype(std::forward<Operation>(operation)())
  {
    try
    {
      return std::forward<Operation>(operation)();
    }
    catch (StorageException const&)
    {
      throw;
    }
    catch (Core::RequestFailedException& error)
    {
      if (!error.RawResponse)
      {
        throw;
      }
      throw StorageException::CreateFromResponse(std::move(error.RawResponse));
    }
  }

}}}} // namespace Azure::Storage::Queues::_detail
