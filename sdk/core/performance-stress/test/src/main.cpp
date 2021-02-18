// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance_framework.hpp>

#include "azure/performance-stress/test/delay_test.hpp"
#include "azure/performance-stress/test/extended_options_test.hpp"
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/performance-stress/test/curl_http_client_get_test.hpp"
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/performance-stress/test/win_http_client_get_test.hpp"
#endif
#include "azure/performance-stress/test/exception_test.hpp"
#include "azure/performance-stress/test/no_op_test.hpp"

#include <vector>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::PerformanceStress::TestMetadata> tests{
      Azure::PerformanceStress::Test::NoOp::GetTestMetadata(),
      Azure::PerformanceStress::Test::ExtendedOptionsTest::GetTestMetadata(),
      Azure::PerformanceStress::Test::DelayTest::GetTestMetadata(),
      Azure::PerformanceStress::Test::ExceptionTest::GetTestMetadata()};

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  tests.emplace_back(Azure::PerformanceStress::Test::CurlHttpClientGetTest::GetTestMetadata());
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  tests.emplace_back(Azure::PerformanceStress::Test::WinHttpClientGetTest::GetTestMetadata());
#endif

  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
