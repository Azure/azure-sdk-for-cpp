// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/program.hpp>

#include "azure/performance-stress/test/delay_test.hpp"
#include "azure/performance-stress/test/extended_options_test.hpp"
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/performance-stress/test/curl_http_client_get_test.hpp"
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/performance-stress/test/win_http_client_get_test.hpp"
#endif
#include "azure/performance-stress/test/no_op_test.hpp"

#include <functional>
#include <iostream>
#include <map>

int main(int argc, char** argv)
{

  // Create the test list
  std::map<
      std::string,
      std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
          Azure::PerformanceStress::TestOptions)>>
      tests{
          {"noOp",
           [](Azure::PerformanceStress::TestOptions options) {
             // No Op
             return std::make_unique<Azure::PerformanceStress::Test::NoOp>(options);
           }},
          {"extendedOptions",
           [](Azure::PerformanceStress::TestOptions options) {
             // Another test
             return std::make_unique<Azure::PerformanceStress::Test::ExtendedOptionsTest>(options);
           }},
          {"delay", [](Azure::PerformanceStress::TestOptions options) {
             // Another test
             return std::make_unique<Azure::PerformanceStress::Test::DelayTest>(options);
           }}};

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  tests.emplace("curlHttpClientGet", [](Azure::PerformanceStress::TestOptions options) {
    // Another test
    return std::make_unique<Azure::PerformanceStress::Test::CurlHttpClientGetTest>(options);
  });
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  tests.emplace("winHttpClientGet", [](Azure::PerformanceStress::TestOptions options) {
    // Another test
    return std::make_unique<Azure::PerformanceStress::Test::WinHttpClientGetTest>(options);
  });
#endif

  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
