// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_per_retry_policy.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/platform.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Details {

  std::unique_ptr<Core::Http::RawResponse> StoragePerRetryPolicy::Send(
      Core::Context const& ctx,
      Core::Http::Request& request,
      std::vector<std::unique_ptr<HttpPolicy>>::const_iterator nextPolicy) const
  {
    const char* HttpHeaderDate = "Date";
    const char* HttpHeaderXMsDate = "x-ms-date";

    const auto& headers = request.GetHeaders();
    if (headers.find(HttpHeaderDate) == headers.end())
    {
      // add x-ms-date header in RFC1123 format
      request.AddHeader(
          HttpHeaderXMsDate,
          Core::DateTime(std::chrono::system_clock::now())
              .ToString(Azure::Core::DateTime::DateFormat::Rfc1123));
    }

    return HttpPolicy::SendNext(ctx, request, nextPolicy);
  }

}}} // namespace Azure::Storage::Details
