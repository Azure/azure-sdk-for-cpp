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
#include <http/curl/curl_session_private.hpp>

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
  TEST_P(TransportAdapter, connectionPoolTest)
  {
    Azure::Core::Http::CurlConnectionPool::ClearIndex();
    // Make sure there are nothing in the pool
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 0);

    // Use the same request for all connections.
    Azure::Core::Http::Request req(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Http::Url("http://httpbin.org/get"));
    std::string const expectedConnectionKey = "httpbin.org0011";

    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      auto connection = Azure::Core::Http::CurlConnectionPool::GetCurlConnection(req, options);
      EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          req, std::move(connection), options.HttpKeepAlive);
      // Simulate connection was used already
      session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
      session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
    }
    // Check that after the connection is gone, it is moved back to the pool
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);
    auto connectionFromPool
        = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin()->second.begin()->get();
    EXPECT_EQ(connectionFromPool->GetConnectionKey(), expectedConnectionKey);

    // Test that asking a connection with same config will re-use the same connection
    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      auto connection = Azure::Core::Http::CurlConnectionPool::GetCurlConnection(req, options);
      // There was just one connection in the pool, it should be empty now
      EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 0);
      // And the connection key for the connection we got is the expected
      EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          req, std::move(connection), options.HttpKeepAlive);
      // Simulate connection was used already
      session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
      session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
    }
    // Check that after the connection is gone, it is moved back to the pool
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);
    auto values = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin();
    EXPECT_EQ(values->second.size(), 1);
    EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);

    // Now test that using a different connection config won't re-use the same connection
    std::string const CAinfo = "someFakePath";
    std::string const secondExpectedKey = "httpbin.org" + CAinfo + "011";
    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      options.CAInfo = CAinfo;
      auto connection = Azure::Core::Http::CurlConnectionPool::GetCurlConnection(req, options);
      EXPECT_EQ(connection->GetConnectionKey(), secondExpectedKey);
      // One connection still in the pool after getting a new connection and with first expected key
      EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);
      EXPECT_EQ(
          Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin()
              ->second.begin()
              ->get()
              ->GetConnectionKey(),
          expectedConnectionKey);

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          req, std::move(connection), options.HttpKeepAlive);
      // Simulate connection was used already
      session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
      session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
    }

    // Now there should be 2 index wit one connection each
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 2);
    values = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin();
    EXPECT_EQ(values->second.size(), 1);
    EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);
    values++;
    EXPECT_EQ(values->second.size(), 1);
    EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), secondExpectedKey);

    // Test re-using same custom config
    {
      // Creating a new connection with default options
      Azure::Core::Http::CurlTransportOptions options;
      options.CAInfo = CAinfo;
      auto connection = Azure::Core::Http::CurlConnectionPool::GetCurlConnection(req, options);
      EXPECT_EQ(connection->GetConnectionKey(), secondExpectedKey);
      // One connection still in the pool after getting a new connection and with first expected key
      EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 1);
      EXPECT_EQ(
          Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin()
              ->second.begin()
              ->get()
              ->GetConnectionKey(),
          expectedConnectionKey);

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          req, std::move(connection), options.HttpKeepAlive);
      // Simulate connection was used already
      session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
      session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
    }
    // Now there should be 2 index wit one connection each
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 2);
    values = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin();
    EXPECT_EQ(values->second.size(), 1);
    EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);
    values++;
    EXPECT_EQ(values->second.size(), 1);
    EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), secondExpectedKey);

#ifdef RUN_LONG_UNIT_TESTS
    {
      // Test pool clean routine
      std::cout << "Running Connection Pool Cleaner Test. This test can take up to 2 minutes to "
                   "complete."
                << std::endl
                << "Add compiler option -DRUN_LONG_UNIT_TESTS=OFF when building if you want to "
                   "skip this test."
                << std::endl;

      // Wait for 100 secs to make sure connections are removed.
      // Connection need to be in the pool for more than 60 sec to consider it expired.
      // Clean routine runs every 90 secs.
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 100));

      // Ensure connections are removed but indexes are still there
      EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 2);
      values = Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.begin();
      EXPECT_EQ(values->second.size(), 0);
      values++;
      EXPECT_EQ(values->second.size(), 0);
    }
#endif
  }

  /*********************** Base Transporter Adapter Tests ******************************/
  INSTANTIATE_TEST_SUITE_P(
      TransportAdapterCurlImpl,
      TransportAdapter,
      testing::Values(GetTransportOptions()),
      GetSuffix);

}}} // namespace Azure::Core::Test
