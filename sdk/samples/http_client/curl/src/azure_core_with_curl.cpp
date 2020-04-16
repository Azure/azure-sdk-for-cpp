// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 *
 */

#include <http/http.hpp>
#include <http/policy.hpp>

#include <iostream>
#include <utility>

using namespace Azure::Core;
using namespace std;

int main()
{
  string host("https://httpbin.org/get");
  cout << "testing curl from transport" << endl << "Host: " << host << endl;

  auto transport = std::make_unique <Http::HttpTransport>();

  // Ability for customers to pass array of Policies
  //  We define order of wiring, perTry and perRetry

  auto requestIdPolicyOptions = Http::RequestIdPolicyOptions();
  auto retry = std::make_unique<Http::RequestIdPolicy>(std::move(transport));
  auto pipeline = std::make_unique<Http::RequestIdPolicy>(std::move(retry));

  auto request = Http::Request(Http::HttpMethod::GET, host);
  auto context = Context();

  try
  {
    auto response = pipeline->Process(context, request);
    cout << response.getReasonPhrase();
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
