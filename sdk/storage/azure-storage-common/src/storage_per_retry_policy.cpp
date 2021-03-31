// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_per_retry_policy.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/platform.hpp>

#include <algorithm>
#include <chrono>

namespace Azure { namespace Storage { namespace _internal {

  std::unique_ptr<Core::Http::RawResponse> StoragePerRetryPolicy::Send(
      Core::Http::Request& request,
      Core::Http::Policies::NextHttpPolicy nextHttpPolicy,
      Core::Context const& ctx) const
  {
    const char* HttpHeaderDate = "Date";
    const char* HttpHeaderXMsDate = "x-ms-date";

    const auto& headers = request.GetHeaders();
    if (headers.find(HttpHeaderDate) == headers.end())
    {
      // add x-ms-date header in RFC1123 format
      request.SetHeader(
          HttpHeaderXMsDate,
          DateTime(std::chrono::system_clock::now())
              .ToString(Azure::DateTime::DateFormat::Rfc1123));
    }

    const char* HttpHeaderTimeout = "timeout";
    auto cancelTimepoint = ctx.GetDeadline();
    if (cancelTimepoint == Azure::DateTime::max())
    {
      request.GetUrl().RemoveQueryParameter(HttpHeaderTimeout);
    }
    else
    {
      auto currentTimepoint = std::chrono::system_clock::now();
      int64_t numSeconds
          = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::time_point(cancelTimepoint) - currentTimepoint)
                .count();
      request.GetUrl().AppendQueryParameter(
          HttpHeaderTimeout, std::to_string(std::max(numSeconds, int64_t(1))));
    }
    return nextHttpPolicy.Send(request, ctx);
  }

}}} // namespace Azure::Storage::_internal
