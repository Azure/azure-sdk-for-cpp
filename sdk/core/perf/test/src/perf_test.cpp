//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/perf/test/delay_test.hpp"
#include "azure/perf/test/extended_options_test.hpp"
#include "azure/perf/test/http_pipeline_get_test.hpp"
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/perf/test/curl_http_client_get_test.hpp"
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/perf/test/win_http_client_get_test.hpp"
#endif
#include "azure/perf/test/exception_test.hpp"
#include "azure/perf/test/no_op_test.hpp"

#include <vector>

int main(int argc, char** argv)
{
  std::cout << "AZURE-CORE-CPP VERSION " << VCPKG_CORE_VERSION << std::endl;
  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Perf::Test::NoOp::GetTestMetadata(),
      Azure::Perf::Test::ExtendedOptionsTest::GetTestMetadata(),
      Azure::Perf::Test::DelayTest::GetTestMetadata(),
      Azure::Perf::Test::ExceptionTest::GetTestMetadata(),
      Azure::Perf::Test::HttpPipelineGetTest::GetTestMetadata(),
  };

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  tests.emplace_back(Azure::Perf::Test::CurlHttpClientGetTest::GetTestMetadata());
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  tests.emplace_back(Azure::Perf::Test::WinHttpClientGetTest::GetTestMetadata());
#endif

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}
