// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Validates the Azure Core transport adapters with fault responses from server.
 *
 * @note This test requires the Http-fault-injector
 * (https://github.com/Azure/azure-sdk-tools/tree/main/tools/http-fault-injector) running. Follow
 * the instructions to install and run the server before running this test.
 *
 */

#define REQUESTS 100
#define WARMUP 100
#define ROUNDS 100

#include <azure/core.hpp>
#include <azure/core/http/curl_transport.hpp>
#include <iostream>

void SendRequest(std::string target)
{
  std::cout << target << std::endl;
  // The transport adapter must allow insecure SSL certs.
  Azure::Core::Http::CurlTransportOptions curlOptions;
  curlOptions.SslVerifyPeer = false;
  auto implementationClient = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);

  try
  {

    Azure::Core::Context context;
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(target));
    auto response = implementationClient->Send(request, context);
    // Make sure to pull all bytes from network.
    auto body = response->ExtractBodyStream()->ReadToEnd();
  }
  catch (std::exception const&)
  {
    // don't print exceptions, they are happening at each request, this is the point of the test
  }
}

void Operation(int repetitions)
{
  std::string base = "https://xyz.";
  for (int i = 0; i < repetitions; i++)
  {
    std::cout << i << std::endl;
    SendRequest(base + std::to_string(i) + ".abc");
  }
}

int main(int argc, char**)
{
  // some param was passed to the program, doesn't matter what it is,
  // it is meant for the moment to just run a quick iteration to check for sanity of the test.
  // since prototype TODO: pass in warmup/rounds/requests as params.
  if (argc != 1)
  {
    std::cout << "--------------\tBUILD TEST\t--------------" << std::endl;
    Operation(5);
    std::cout << "--------------\tEND BUILD TEST\t--------------" << std::endl;
    return 0;
  }

  std::cout << "--------------\tSTARTING TEST\t--------------" << std::endl;
  std::cout << "--------------\tPRE WARMUP\t--------------" << std::endl;
  Operation(WARMUP);

  std::cout << "--------------\tPOST WARMUP\t--------------" << std::endl;

  for (int i = 0; i < ROUNDS; i++)
  {
    std::cout << "--------------\tTEST ITERATION:" << i << "\t--------------" << std::endl;
    Operation(REQUESTS);

    std::cout << "--------------\tDONE ITERATION:" << i << "\t--------------" << std::endl;
  }

  return 0;
}
