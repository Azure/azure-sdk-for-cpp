#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/policy.hpp"

#include <curl/curl.h>
#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class CurlTransport : public HttpTransport {
  private:
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

  // stream to be returned inside HTTP response when using curl
  // It keeps the ref to CurlTrasnport in order to close the handle once done
  class CurlBodyStream : public Azure::Core::Http::BodyStream {
  private:
    uint64_t m_length;
    CurlTransport const& m_curlAdapter;

  public:
    // length comes from http response header `content-length`
    CurlBodyStream(uint64_t length, CurlTransport const& adapter)
        : m_length(length), m_curlAdapter(adapter)
    {
    }

    uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t count)
    {
      // Read bytes from curl into buffer.
      (void)buffer;
      (void)count;
      return 0;
    }

    void Close(){
        // CurlTransport de-constructor takes care of curl cleaning.
        // Should we move it to be explicitly requested by user instead?
    };
  };
}}} // namespace Azure::Core::Http
