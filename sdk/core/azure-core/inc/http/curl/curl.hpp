#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/policy.hpp"

#include <chrono>
#include <curl/curl.h>
#include <thread>
#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class CurlTransport : public HttpTransport {
  private:
    // for every client instance, create a default response
    std::unique_ptr<Azure::Core::Http::Response> m_response;
    bool m_isFirstHeader;
    bool m_isFirstBodyCallBack;
    // initial state of reader is always pause. It will wait for user to request a read to
    // un-pause.
    bool m_isPausedRead = true;
    bool m_isStreamRequest; // makes transport to return stream in response
    bool m_isPullCompleted;
    void* m_responseUserBuffer;
    uint64_t m_responseContentLength;
    Azure::Core::Http::Request* m_request;

    CURL* m_pCurl;

    static size_t WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t WriteBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t ReadBodyCallBack(void* dst, size_t size, size_t nmemb, void* userdata);
    static int progressCallback(
        void* clientp,
        curl_off_t dltotal,
        curl_off_t dlnow,
        curl_off_t ultotal,
        curl_off_t ulnow);

    bool isUploadRequest()
    {
      return this->m_request->GetMethod() == HttpMethod::Put
          || this->m_request->GetMethod() == HttpMethod::Post;
    }

    // setHeaders()
    CURLcode SetUrl()
    {
      return curl_easy_setopt(m_pCurl, CURLOPT_URL, this->m_request->GetEncodedUrl().c_str());
    }

    CURLcode SetHeaders()
    {
      auto headers = this->m_request->GetHeaders();
      if (headers.size() == 0)
      {
        return CURLE_OK;
      }

      // creates a slist for bulding curl headers
      struct curl_slist* headerList = NULL;

      // insert headers
      for (auto header : headers)
      {
        headerList = curl_slist_append(headerList, (header.first + ":" + header.second).c_str());
        if (headerList == NULL)
        {
          throw;
        }
      }

      if (isUploadRequest())
      {
        // set expect header for PUT and POST request. This disables libcurl to send only headers
        // and expect sever to return a Continue respond before posting body
        headerList = curl_slist_append(headerList, "Expect:");
        // inf header for payload size
        auto size = this->m_request->GetBodyStream()->Length(); //
        auto result = curl_easy_setopt(m_pCurl, CURLOPT_INFILESIZE, (curl_off_t)size);
        if (result != CURLE_OK)
        {
          throw;
        }
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

      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_XFERINFOFUNCTION, progressCallback);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_XFERINFODATA, (void*)this);

      return curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void*)this);
    }

    // set callback for puting data into libcurl
    CURLcode SetReadRequest()
    {
      auto settingUp = curl_easy_setopt(m_pCurl, CURLOPT_UPLOAD, 1L);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, ReadBodyCallBack);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }
      settingUp = curl_easy_setopt(m_pCurl, CURLOPT_READDATA, (void*)this);
      if (settingUp != CURLE_OK)
      {
        return settingUp;
      }

      return curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void*)this);
    }

    CURLcode Perform(Context& context);

    void ParseHeader(std::string const& header);

  public:
    CurlTransport();
    ~CurlTransport();

    std::unique_ptr<Response> Send(Context& context, Request& request) override;

    // using this function we can change the buffer where libcurl will write from wire
    void SetBodyCallBackBuffer(uint8_t* buffer) { m_responseUserBuffer = buffer; }

    // un-pause libcurl to read and pull data from wire
    uint64_t PullData()
    {
      this->m_isPullCompleted = false;
      curl_easy_pause(m_pCurl, CURLPAUSE_CONT);
      while (!m_isPullCompleted)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
      return 0;
    }
  }; // namespace Http

  // stream to be returned inside HTTP response when using curl
  // It keeps the ref to CurlTrasnport in order to close the handle once done
  class CurlBodyStream : public Azure::Core::Http::BodyStream {
  private:
    uint64_t m_length;
    CurlTransport* m_curlAdapter;

  public:
    // length comes from http response header `content-length`
    CurlBodyStream(uint64_t length, CurlTransport* adapter)
        : m_length(length), m_curlAdapter(adapter)
    {
    }

    uint64_t Length() { return m_length; }

    uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t count)
    {
      // Read bytes from curl into buffer.
      (void)count;

      // Set buffer as the destination to write
      this->m_curlAdapter->SetBodyCallBackBuffer(buffer);

      // pullData
      return this->m_curlAdapter->PullData();
    }

    void Close(){
        // CurlTransport de-constructor takes care of curl cleaning.
        // Should we move it to be explicitly requested by user instead?
    };
  };
}}} // namespace Azure::Core::Http
