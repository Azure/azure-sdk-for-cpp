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

  try
  {
    auto response = http::Client::send(request);
    cout << response.getHttpVersion() << '\n';
    cout << response.getStatusCode() << '\n';
    cout << response.getReasonPhrase() << '\n';
    cout << "headers:" << '\n';
    for (auto header : response.getHeaders())
    {
      cout << header.first << " : " << header.second << '\n';
    }
    cout << "Body (buffer):" << '\n';
    cout << response.getBodyBuffer() << '\n';
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
