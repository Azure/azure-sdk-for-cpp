// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

namespace Azure { namespace Storage { namespace Details {

  std::unique_ptr<Azure::Core::Http::RawResponse> StorageSwitchToSecondaryPolicy::Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::NextHttpPolicy nextHttpPolicy,
      const Azure::Core::Context& ctx) const
  {
    bool considerSecondary = (request.GetMethod() == Azure::Core::Http::HttpMethod::Get
                              || request.GetMethod() == Azure::Core::Http::HttpMethod::Head)
        && !m_secondaryHost.empty();

    int retryNumber = Azure::Core::Http::RetryPolicy::GetRetryNumber(ctx);
    if (retryNumber > 0)
    {
      // switch host
      if (request.GetUrl().GetHost() == m_primaryHost && considerSecondary)
      {
        request.GetUrl().SetHost(m_secondaryHost);
      }
      else
      {
        request.GetUrl().SetHost(m_primaryHost);
      }
    }

    auto response = nextHttpPolicy.Send(request, ctx);

    if (response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::NotFound
        || response->GetStatusCode() == Core::Http::HttpStatusCode::PreconditionFailed)
    {
      // FIXME: need a machenism to retain this value on a per-operation basis.
      considerSecondary = false;
    }

    return response;
  }

}}} // namespace Azure::Storage::Details
