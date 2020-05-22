#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/http_client.hpp"
#include "http/policy.hpp"

#include <curl/curl.h>
#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class CurlTransport : public Transport {
  private:
    CURL* m_p_curl;

      // setHeaders()
    CURLcode setUrl(Request& request)
    {
      return curl_easy_setopt(m_p_curl, CURLOPT_URL, request.getEncodedUrl().c_str());
    }

    CURLcode perform(Request& request)
    {
      auto settingUp = setUrl(request);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      return curl_easy_perform(m_p_curl);
    }

  public:
    CurlTransport();
    ~CurlTransport();

    Response Send(Context& context, Request& request) override;
  };

}}} // namespace Azure::Core::Http
