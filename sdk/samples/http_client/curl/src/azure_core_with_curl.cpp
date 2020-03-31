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

  try
  {
    auto response = http::Client::send(request);
    cout << response.getReasonPhrase();
  }
  catch (http::CouldNotResolveHostException& e)
  {
    cout << e.what() << endl;
  }
  catch (const char* msg)
  {
    cout << msg << endl;
  }

  return 0;
}
