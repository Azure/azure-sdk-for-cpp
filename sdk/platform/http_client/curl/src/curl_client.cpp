// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <curl_client.hpp>
#include <http/http.hpp>

#include <iostream>

using namespace azure::core::http;
using namespace std;

Response CurlClient::send()
{
  auto performing = perform();

  if (performing != CURLE_OK)
  {
    switch (performing)
    {
      case CURLE_COULDNT_RESOLVE_HOST:
      {
        throw azure::core::http::CouldNotResolveHostException();
      }
      case CURLE_WRITE_ERROR:
      {
        throw azure::core::http::ErrorWhileWrittingResponse();
      }
      default:
      {
        throw azure::core::http::TransportException();
      }
    }
  }

  return m_response;
}

CURLcode CurlClient::perform()
{
  // init instance state
  m_response = azure::core::http::Response();
  m_firstHeader = true;

  auto settingUp = setUrl();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  settingUp = setWriteResponse();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  return curl_easy_perform(m_pCurl);
}

static void parseAndSetFirstHeader(std::string const& header, Response& response)
{
  // set response code, http version and reason phrase (i.e. HTTP/1.1 200 OK)
  auto start = std::find(header.begin(), header.end(), '/');
  start++; // skip symbol
  auto end = std::find(start, header.end(), '.');
  auto mayorVersion = std::stoi(std::string(start, end));

  start = end + 1; // start of minor version
  end = std::find(start, header.end(), ' ');
  auto minorVersion = std::stoi(std::string(start, end));

  start = end + 1; // start of status code
  end = std::find(start, header.end(), ' ');
  auto statusCode = std::stoi(std::string(start, end));

  start = end + 1; // start of reason phrase
  auto reasonPhrase = std::string(start, header.end() - 2); // remove \r and \n from the end

  response.setVersion(mayorVersion, minorVersion);
  response.setStatusCode(statusCode);
  response.setReasonPhrase(reasonPhrase);
}

static void parseHeader(std::string const& header, Response& response)
{
  // get name and value from header
  auto start = header.begin();
  auto end = std::find(start, header.end(), ':');

  if (end == header.end())
  {
    return; // not a valid header or end of headers symbol reached
  }

  auto headerName = std::string(start, end);
  start = end + 1; // start value

  auto headerValue = std::string(start, header.end() - 2); // remove \r and \n from the end

  response.addHeader(headerName, headerValue);
}

size_t CurlClient::writeCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
  size_t const expected_size = size * nmemb;

  // cast client
  CurlClient* client = (CurlClient*)userp;
  // convert response to standar string
  std::string response = std::string((char*)contents, expected_size);

  if (client->m_firstHeader)
  {
    // first header is expected to be the status code, version and reasonPhrase
    parseAndSetFirstHeader(response, client->m_response);
    client->m_firstHeader = false;
    return expected_size;
  }

  // parse all next headers and add them
  parseHeader(response, client->m_response);

  // This callback needs to return the response size or curl will consider it as it failed
  return expected_size;
}
