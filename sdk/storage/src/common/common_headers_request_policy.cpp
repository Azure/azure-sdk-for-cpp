// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/common_headers_request_policy.hpp"

#include <ctime>

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
      static const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
      static const char* months[]
          = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
      std::string rfc1123Format = "%a, %d %b %Y %H:%M:%S GMT";
      rfc1123Format.replace(rfc1123Format.find("%a"), 2, weekdays[ct.tm_wday]);
      rfc1123Format.replace(rfc1123Format.find("%b"), 2, months[ct.tm_mon]);
      char datetimeStr[32];
      std::strftime(datetimeStr, sizeof(datetimeStr), rfc1123Format.data(), &ct);

      request.AddHeader(c_HttpHeaderXMsDate, datetimeStr);
    }

    return nextHttpPolicy.Send(ctx, request);
  }

}} // namespace Azure::Storage
