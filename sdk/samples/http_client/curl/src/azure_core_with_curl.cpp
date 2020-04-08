// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 *
 */

#include <http/http.hpp>

#include <iostream>

using namespace azure::core;
using namespace std;

int main()
{
  string host("https://httpbin.org/get");
  cout << "testing curl from transport" << endl << "Host: " << host << endl;

  auto request = http::Request(http::HttpMethod::GET, host);
  request.addHeader("one", "header");
  request.addHeader("other", "header2");
  request.addHeader("header", "value");

  try
  {
    http::Response* response = http::Client::Send(request);

    if (response == nullptr)
    {
      cout << "Error. Response returned as null";
      return 0;
    }

    cout << static_cast<typename std::underlying_type<http::HttpStatusCode>::type>(
                response->GetStatusCode())
         << '\n';
    cout << response->GetReasonPhrase() << '\n';
    cout << "headers:" << '\n';
    for (auto header : response->GetHeaders())
    {
      cout << header.first << " : " << header.second << '\n';
    }
    cout << "Body (buffer):" << '\n';
    auto bodyVector = response->GetBodyBuffer();
    cout << std::string(bodyVector.begin(), bodyVector.end());

    // free response
    delete response;
  }
  catch (http::CouldNotResolveHostException& e)
  {
    cout << e.what() << endl;
  }
  catch (http::TransportException& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}
