// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/common_headers_request_policy.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace Azure { namespace Storage {

  std::unique_ptr<Core::Http::RawResponse> CommonHeadersRequestPolicy::Send(
      Core::Context const& ctx,
      Core::Http::Request& request,
      Core::Http::NextHttpPolicy nextHttpPolicy) const
  {
    const char* c_HttpHeaderDate = "Date";
    const char* c_HttpHeaderXMsDate = "x-ms-date";

    const auto& headers = request.GetHeaders();
    if (headers.find(c_HttpHeaderDate) == headers.end())
    {
      // add x-ms-date header in RFC1123 format
      // TODO: call helper function provided by Azure Core when they provide one.
      time_t t = std::time(nullptr);
      struct tm ct;
#ifdef _WIN32
      gmtime_s(&ct, &t);
#else
      gmtime_r(&t, &ct);
#endif
      std::stringstream dateString;
      dateString.imbue(std::locale("C"));
      dateString << std::put_time(&ct, "%a, %d %b %Y %H:%M:%S GMT");
      request.AddHeader(c_HttpHeaderXMsDate, dateString.str());
    }

    return nextHttpPolicy.Send(ctx, request);
  }

}} // namespace Azure::Storage
