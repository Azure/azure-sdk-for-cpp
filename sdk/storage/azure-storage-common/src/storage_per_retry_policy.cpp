// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_per_retry_policy.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/platform.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Details {

  std::unique_ptr<Core::Http::RawResponse> StoragePerRetryPolicy::Send(
      Core::Http::Request& request,
      Core::Http::NextHttpPolicy nextHttpPolicy,
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
          Core::DateTime(std::chrono::system_clock::now())
              .ToString(Azure::Core::DateTime::DateFormat::Rfc1123));
    }

    return nextHttpPolicy.Send(request, ctx);
  }

}}} // namespace Azure::Storage::Details
