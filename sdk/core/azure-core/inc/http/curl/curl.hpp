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
    CurlTransport(const CurlTransport&) = delete;
    CurlTransport& operator=(const CurlTransport&) = delete;

    // for every client instance, create a default response
    std::unique_ptr<Azure::Core::Http::Response> m_response;
    bool m_firstHeader;

    CURL* m_pCurl;

    static size_t WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t WriteBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp);

    // setHeaders()
    CURLcode SetUrl(Request& request)
    {
      return curl_easy_setopt(m_pCurl, CURLOPT_URL, request.GetEncodedUrl().c_str());
    }

    CURLcode SetHeaders(Request& request)
    {
      auto headers = request.GetHeaders();
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

    CURLcode SetWriteResponse()
    {
      auto settingUp = curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, WriteHeadersCallBack);
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
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteBodyCallBack);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }

      return curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void*)this);
    }

    CURLcode Perform(Context& context, Request& request);

  public:
    CurlTransport();
    ~CurlTransport();

    std::unique_ptr<Response> Send(Context& context, Request& request) override;
  };

}}} // namespace Azure::Core::Http
