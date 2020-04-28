// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <curl/curl.h>
#include <http/http.hpp>

class CurlClient
{
private:
  azure::core::http::Request& m_request;
  curl_slist* m_slist = nullptr;
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
    if (m_request.getMethod() == azure::core::http::HttpMethod::Put)
    {
      curl_easy_setopt(m_p_curl, CURLOPT_PUT, 1L);
    }

    for (const auto header : m_request.getHeaders())
    {
      add_header(header.first, header.second);
    }
    if (m_request.getBodyBuffer() != nullptr)
    {
      add_header("Content-Length", std::to_string(m_request.getBodyBuffer()->_bodyBufferSize));
      add_header("Transfer-Encoding", std::string());
      curl_easy_setopt(m_p_curl, CURLOPT_READFUNCTION, read);
      curl_easy_setopt(m_p_curl, CURLOPT_READDATA, m_request.getBodyBuffer());
      curl_easy_setopt(m_p_curl, CURLOPT_TRANSFER_ENCODING, 0); // Disable transfer encoding.
    }
    curl_easy_setopt(m_p_curl, CURLOPT_HTTPHEADER, m_slist);

    //curl_easy_setopt(m_p_curl, CURLOPT_PROXY, "127.0.0.1:8888"); curl_easy_setopt(m_p_curl, CURLOPT_SSL_VERIFYPEER, false); // Fiddler
    return curl_easy_perform(m_p_curl);
  }

  void add_header(const std::string& name, const std::string& value)
  {
    std::string header(name);
    header.append(": ").append(value);
    m_slist = curl_slist_append(m_slist, header.data());
  }

  static size_t read(char* buffer, size_t size, size_t nitems, void* userdata)
  {
    azure::core::http::BodyBuffer* inputBuffer
        = static_cast<azure::core::http::BodyBuffer*>(userdata);
    size_t contentlen = inputBuffer->_bodyBufferSize - inputBuffer->_currentPos;
    size_t actual_size = min(contentlen, size * nitems);

    if (inputBuffer->_bodyBuffer != NULL)
    {
      memcpy(buffer, inputBuffer->_bodyBuffer + inputBuffer->_currentPos, actual_size);
      inputBuffer->_currentPos += actual_size;
    }

    return actual_size;
  }

public:
  CurlClient(azure::core::http::Request& request) : m_request(request)
  {
    m_p_curl = curl_easy_init();
  }
  // client curl struct on destruct
  ~CurlClient()
  {
    curl_easy_cleanup(m_p_curl);
    if (m_slist)
    {
      curl_slist_free_all(m_slist);
    }
  }

  azure::core::http::Response send();
};
