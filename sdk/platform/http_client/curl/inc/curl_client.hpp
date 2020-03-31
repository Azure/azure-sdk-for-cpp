// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <curl/curl.h>
#include <http/http.hpp>

class CurlClient
{
private:
  azure::core::http::Request& _request;
  CURL* _p_curl;

  // setHeaders()
  CURLcode setUrl()
  {
    return curl_easy_setopt(_p_curl, CURLOPT_URL, this->_request.getEncodedUrl().c_str());
  }
  CURLcode perform()
  {
    auto settingUp = setUrl();
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }
    return curl_easy_perform(_p_curl);
  }

public:
  CurlClient(azure::core::http::Request& request) : _request(request)
  {
    _p_curl = curl_easy_init();
  }
  // client curl struct on destruct
  ~CurlClient() { curl_easy_cleanup(_p_curl); }

  azure::core::http::Response send();
};
