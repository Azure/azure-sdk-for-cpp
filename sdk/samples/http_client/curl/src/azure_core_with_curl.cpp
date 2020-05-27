// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 *
 */

#include <http/http.hpp>
#include <http/http_client.hpp>
#include <http/curl/curl.hpp>

#include <iostream>
#include <memory>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

int main()
{
  string host("https://httpbin.org/get");
  cout << "testing curl from transport" << endl << "Host: " << host << endl;

  try
  {
    auto request = Http::Request(Http::HttpMethod::Get, host);
    request.AddHeader("one", "header");
    request.AddHeader("other", "header2");
    request.AddHeader("header", "value");

    auto httpClientOptions = Http::HttpClientOptions();
    httpClientOptions.Transport = Http::TransportKind::Curl;

    auto httpClient = Http::HttpClient(httpClientOptions);

    auto context = Context();
    std::shared_ptr<Http::Response> response = httpClient.Send(context, request);

    if (response == nullptr)
    {
      cout << "Error. Response returned as null";
      return 0;
    }

    cout << static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
        response->GetStatusCode())
         << endl;
    cout << response->GetReasonPhrase() << endl;
    cout << "headers:" << endl;
    for (auto header : response->GetHeaders())
    {
      cout << header.first << " : " << header.second << endl;
    }
    cout << "Body (buffer):" << endl;
    auto bodyVector = response->GetBodyBuffer();
    cout << std::string(bodyVector.begin(), bodyVector.end());
  }
  catch (Http::CouldNotResolveHostException& e)
  {
    cout << e.what() << endl;
  }
  catch (Http::TransportException& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}
