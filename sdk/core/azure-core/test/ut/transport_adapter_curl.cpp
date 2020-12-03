// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter_base.hpp"
#include <azure/core/context.hpp>
#include <azure/core/response.hpp>
#include <iostream>
#include <string>
#include <thread>

#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>

using testing::ValuesIn;

namespace Azure { namespace Core { namespace Test {

  /**********************   Define the parameters for the base test and a suffix  ***************/
  namespace {
    static Azure::Core::Http::TransportPolicyOptions GetTransportOptions()
    {
      Azure::Core::Http::TransportPolicyOptions options;
      options.Transport = std::make_shared<Azure::Core::Http::CurlTransport>();
      return options;
    }

    // When adding more than one parameter, this function should return a unique string.
    // But since we are only using one parameter (the libcurl transport adapter) this is fine.
    static std::string GetSuffix(const testing::TestParamInfo<TransportAdapter::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#specifying-names-for-value-parameterized-test-parameters
      (void)(info);
      std::string suffix("curlImplementation");
      return suffix;
    }
  } // namespace

  /***********************  Unique Tests for Libcurl   ********************************/
  TEST_P(TransportAdapter, DISABLED_connectionPoolTest)
  {
    Azure::Core::Http::Url host("http://httpbin.org/get");
    Azure::Core::Http::CurlConnectionPool::ClearIndex();

    auto threadRoutine = [&]() {
      using namespace std::chrono_literals;
      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
      auto response = m_pipeline->Send(Azure::Core::GetApplicationContext(), request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
      std::this_thread::sleep_for(1s);
    };

    std::thread t1(threadRoutine);
    std::thread t2(threadRoutine);
    t1.join();
    t2.join();

    // 2 connections must be available at this point
    EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 2);

    std::thread t3(threadRoutine);
    std::thread t4(threadRoutine);
    std::thread t5(threadRoutine);
    t3.join();
    t4.join();
    t5.join();

    // Two connections re-used plus one connection created
    EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 3);

#ifdef RUN_LONG_UNIT_TESTS
    {
      // Test pool clean routine
      std::cout << "Running Connection Pool Cleaner Test. This test takes more than 3 minutes to "
                   "complete."
                << std::endl
                << "Add compiler option -DRUN_LONG_UNIT_TESTS=OFF when building if you want to "
                   "skip this test."
                << std::endl;

      // Wait for 180 secs to make sure any previous connection is removed by the cleaner
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 180));

      std::cout << "First wait time done. Validating state." << std::endl;

      // index is not affected by cleaner. It does not remove index
      EXPECT_EQ(Http::CurlConnectionPool::ConnectionsIndexOnPool(), 1);
      // cleaner should have removed connections
      EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 0);

      std::thread t1(threadRoutine);
      std::thread t2(threadRoutine);
      t1.join();
      t2.join();

      // wait for connection to be moved back to pool
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      // 2 connections must be available at this point and one index
      EXPECT_EQ(Http::CurlConnectionPool::ConnectionsIndexOnPool(), 1);
      // Depending on how fast the previous requests are sent, there could be one or more
      // connections in the pool. If first request is too fast, the second request will reuse the
      // same connection.
      EXPECT_PRED1(
          [](int currentConnections) { return currentConnections > 1; },
          Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"));
    }
#endif
  }

  /*********************** Base Transpoer Adapter Tests ******************************/
  INSTANTIATE_TEST_SUITE_P(
      TransportAdapterCurlImpl,
      TransportAdapter,
      testing::Values(GetTransportOptions()),
      GetSuffix);

}}} // namespace Azure::Core::Test
