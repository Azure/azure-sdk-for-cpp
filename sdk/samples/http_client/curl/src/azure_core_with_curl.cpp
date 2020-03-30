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
  cout << "testing curl from transport...\n";

  auto request
      = http::Request(http::HttpMethod::GET, "http://www.cplusplus.com/doc/tutorial/basic_io/");

  auto response = http::Client::send(request);

  cout << "end";

  return 0;
}