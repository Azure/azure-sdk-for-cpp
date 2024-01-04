// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/http/policies/per_retry_policy.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/platform.hpp>

#include <algorithm>
#include <chrono>

namespace Azure { namespace Core { namespace Http { namespace Policies { namespace _internal {

  std::unique_ptr<Core::Http::RawResponse> PerRetryPolicy::Send(
      Core::Http::Request& request,
      Core::Http::Policies::NextHttpPolicy nextPolicy,
      Core::Context const& context) const
  {
    const auto& headers = request.GetHeaders();
    if (headers.find(HttpHeaderDate) == headers.end())
    {
      // add x-ms-date header in RFC1123 format
      request.SetHeader(
          HttpHeaderXMsDate,
          DateTime(std::chrono::system_clock::now())
              .ToString(Azure::DateTime::DateFormat::Rfc1123));
    }

    auto cancelTimepoint = context.GetDeadline();
    if (cancelTimepoint == Azure::DateTime::max())
    {
      request.GetUrl().RemoveQueryParameter(HttpQueryTimeout);
    }
    else
    {
      auto currentTimepoint = DateTime(std::chrono::system_clock::now());
      int64_t numSeconds = (cancelTimepoint > currentTimepoint)
          ? std::chrono::duration_cast<std::chrono::seconds>(cancelTimepoint - currentTimepoint)
                .count()
          : -1;
      request.GetUrl().AppendQueryParameter(
          HttpQueryTimeout, std::to_string(std::max(numSeconds, int64_t(1))));
    }

    return nextPolicy.Send(request, context);
  }

}}}}} // namespace Azure::Core::Http::Policies::_internal
