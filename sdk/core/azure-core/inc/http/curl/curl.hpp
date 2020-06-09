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

  class CurlSession {
  private:
    CURL* m_pCurl;
    std::unique_ptr<Response> m_response;
    Request& m_request;
    size_t uploadedBytes; // controls a bodyBuffer upload

    // Headers
    static size_t WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    // Body from libcurl to httpResponse
    static size_t WriteBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp);
    // Body from httpRequest to libcurl
    static size_t ReadBodyCallBack(void* dst, size_t size, size_t nmemb, void* userdata);

    bool isUploadRequest()
    {
      return this->m_request.GetMethod() == HttpMethod::Put
          || this->m_request.GetMethod() == HttpMethod::Post;
    }

    // setHeaders()
    CURLcode SetUrl()
    {
      return curl_easy_setopt(m_pCurl, CURLOPT_URL, this->m_request.GetEncodedUrl().c_str());
    }

    CURLcode SetMethod()
    {
      HttpMethod method = this->m_request.GetMethod();
      if (method == HttpMethod::Get)
      {
        return curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, 1L);
      }
      else if (method == HttpMethod::Put)
      {
        return curl_easy_setopt(m_pCurl, CURLOPT_PUT, 1L);
      }
      else if (method == HttpMethod::Head)
      {
        return curl_easy_setopt(m_pCurl, CURLOPT_NOBODY, 1L);
      }
      return CURLE_OK;
    }

    CURLcode SetHeaders()
    {
      auto headers = this->m_request.GetHeaders();
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
        auto requestStream = this->m_request.GetBodyStream();
        auto size = requestStream != nullptr ? requestStream->Length()
                                             : this->m_request.GetBodyBuffer().size();
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
      this->uploadedBytes = 0; // restart control counter during setup
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

    void ParseHeader(std::string const& header);

  public:
    CurlSession(Request& request) : m_request(request) { m_pCurl = curl_easy_init(); }

    CURLcode Perform(Context& context);
    std::unique_ptr<Azure::Core::Http::Response> GetResponse() { return std::move(m_response); }
  };

  class CurlTransport : public HttpTransport {
  private:
    bool m_isFirstBodyCallBack = true;
    // initial state of reader is always pause. It will wait for user to request a read to
    // un-pause.
    bool m_isPausedRead = true;
    bool m_isStreamRequest; // makes transport to return stream in response
    bool m_isPullCompleted;
    void* m_responseUserBuffer;
    uint64_t m_responseContentLength;
    Azure::Core::Http::Request* m_request;

  public:
    CurlTransport();
    ~CurlTransport();

    std::unique_ptr<Response> Send(Context& context, Request& request) override;

  }; // namespace Http

  // stream to be returned inside HTTP response when using curl
  // It keeps the ref to CurlTrasnport in order to close the handle once done
  class CurlBodyStream : public Azure::Core::Http::BodyStream {
  private:
    uint64_t m_length;
    std::shared_ptr<CurlSession> m_curlSession;

  public:
    // length comes from http response header `content-length`
    CurlBodyStream(uint64_t length, std::shared_ptr<CurlSession> curlSession)
        : m_length(length), m_curlSession(curlSession)
    {
    }

    uint64_t Length() { return m_length; }

    uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t count)
    {
      // Read bytes from curl into buffer.
      (void)count;
      (void)buffer;

      // TODO: use the culrSession to make a read from socket
      return 0;
    }

    void Close(){
        // call the cleanup from Session
        // Session will be deleted once stream is
    };
  };
}}} // namespace Azure::Core::Http
