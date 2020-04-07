// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <curl/curl.h>
#include <http/http.hpp>

class CurlClient
{
private:
  azure::core::http::Request& m_request;
  // for every client instance, create a default response
  azure::core::http::Response m_response;
  bool m_firstHeader;

  CURL* m_pCurl;

  static size_t writeHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp);
  static size_t writeBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp);

  // setHeaders()
  CURLcode setUrl()
  {
    return curl_easy_setopt(m_pCurl, CURLOPT_URL, this->m_request.getEncodedUrl().c_str());
  }

  CURLcode setHeaders()
  {
    auto headers = this->m_request.getHeaders();
    if (headers.size() == 0)
    {
      return CURLE_OK;
    }

    // creates a slist for bulding curl headers
    struct curl_slist* headerList = NULL;

    // insert headers
    for (auto header : headers)
    {
      // TODO: check result is not null or trow
      headerList = curl_slist_append(headerList, (header.first + ":" + header.second).c_str());
    }

    // set all headers from slist
    return curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headerList);
  }

  CURLcode setWriteResponse()
  {
    auto settingUp = curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, writeHeadersCallBack);
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }
    settingUp = curl_easy_setopt(m_pCurl, CURLOPT_HEADERDATA, (void*)this);
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }
    // TODO: set up cache size. user should be able to set it up
    settingUp = curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, writeBodyCallBack);
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }

    return curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void*)this);
  }

  CURLcode perform();

public:
  CurlClient(azure::core::http::Request& request) : m_request(request)
  {
    m_pCurl = curl_easy_init();
  }
  // client curl struct on destruct
  ~CurlClient() { curl_easy_cleanup(m_pCurl); }

  azure::core::http::Response send();
};
