// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

namespace Azure { namespace Storage { namespace _internal {

  std::unique_ptr<Azure::Core::Http::RawResponse> StorageSwitchToSecondaryPolicy::Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy nextHttpPolicy,
      const Azure::Core::Context& ctx) const
  {
    std::shared_ptr<bool> replicaStatus;
    if (ctx.HasKey(SecondaryHostReplicaStatusKey))
    {
      replicaStatus = ctx.GetValue<std::shared_ptr<bool>>(SecondaryHostReplicaStatusKey);
    }

    bool considerSecondary = (request.GetMethod() == Azure::Core::Http::HttpMethod::Get
                              || request.GetMethod() == Azure::Core::Http::HttpMethod::Head)
        && !m_secondaryHost.empty() && replicaStatus && *replicaStatus;

    if (considerSecondary
        && Azure::Core::Http::Policies::_internal::RetryPolicy::GetRetryNumber(ctx) > 0)
    {
      // switch host
      if (request.GetUrl().GetHost() == m_primaryHost)
      {
        request.GetUrl().SetHost(m_secondaryHost);
      }
      else
      {
        request.GetUrl().SetHost(m_primaryHost);
      }
    }

    auto response = nextHttpPolicy.Send(request, ctx);

    if (considerSecondary
        && (response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::NotFound
            || response->GetStatusCode() == Core::Http::HttpStatusCode::PreconditionFailed)
        && request.GetUrl().GetHost() == m_secondaryHost)
    {
      *replicaStatus = false;
      // switch back
      request.GetUrl().SetHost(m_primaryHost);
      response = nextHttpPolicy.Send(request, ctx);
    }

    return response;
  }

}}} // namespace Azure::Storage::_internal
