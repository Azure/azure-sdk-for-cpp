// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <curl/curl.h>
#include <http/http.hpp>

class CurlClient {
private:
  Azure::Core::Http::Request& m_request;
  CURL* m_p_curl;

  // setHeaders()
  CURLcode setUrl()
  {
    return curl_easy_setopt(m_p_curl, CURLOPT_URL, this->m_request.getEncodedUrl().c_str());
  }
  CURLcode perform()
  {
    auto settingUp = setUrl();
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }
    return curl_easy_perform(m_p_curl);
  }

public:
  CurlClient(Azure::Core::Http::Request& request) : m_request(request)
  {
    m_p_curl = curl_easy_init();
  }
  // client curl struct on destruct
  ~CurlClient()
  {
    curl_easy_cleanup(m_p_curl);
  }

  Azure::Core::Http::Response send();
};
