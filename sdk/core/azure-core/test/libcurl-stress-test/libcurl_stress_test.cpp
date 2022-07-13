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
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>

void SendRequest(std::string target)
{
  std::cout << target << std::endl;
  /* The transport adapter must allow insecure SSL certs.
  If both curl and winHttp are available, curl is preferred for this test.for*/
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  Azure::Core::Http::CurlTransportOptions curlOptions;
  curlOptions.SslVerifyPeer = false;
  auto implementationClient = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);

#elif (BUILD_TRANSPORT_WINHTTP_ADAPTER)
  Azure::Core::Http::WinHttpTransportOptions winHttpOptions;
  auto implementationClient = std::make_shared<Azure::Core::Http::WinHttpTransport>(winHttpOptions);
#endif
  try
  {

    Azure::Core::Context context;
    //  auto duration = std::chrono::milliseconds(1000);
    // auto deadline = std::chrono::system_clock::now() + duration;
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(target));
    auto response = implementationClient->Send(request, context); //.WithDeadline(deadline));
    // Make sure to pull all bytes from network.
    auto body = response->ExtractBodyStream()->ReadToEnd();
  }
  catch (std::exception const&)
  {
  }
}

void Operation(int const& repetitions)
{
  char buffer[12];
  std::string base = "https://";
  for (int i = 0; i < repetitions; i++)
  {
    std::cout << i << std::endl;
    snprintf(buffer, sizeof(buffer), "%d", i);
    SendRequest(base + buffer + ".com" + '\0');
  }
}

int main(int argc, char** argv) // i can have either 0 or 2 params here
{
  (void)argv; // to get rid of the unused warning
  // some param was passed to the program , which happens only in build mode, not run of the docker
  // file. thus we will run a quick test to make sure the executable execuates.
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
