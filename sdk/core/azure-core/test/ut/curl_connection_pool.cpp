// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter_base.hpp"
#include <azure/core/context.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/response.hpp>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"
#endif

#include <iostream>
#include <string>
#include <thread>

// The next includes are from Azure Core private headers.
// That's why the path starts from `sdk/core/azure-core/src/`
// They are included to test the connection pool from the curl transport adapter implementation.
#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

#include "curl_session.hpp"

using testing::ValuesIn;
using namespace Azure::Core::Http::_detail;

namespace Azure { namespace Core { namespace Test {

/***********************  Unique Tests for Libcurl   ********************************/
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)

    TEST(CurlConnectionPool, connectionPoolTest)
    {
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        CurlConnectionPool::g_curlConnectionPool.ResetPool();
      }
      // Make sure there are nothing in the pool
      EXPECT_EQ(CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.size(), 0);

      // Use the same request for all connections.
      Azure::Core::Http::Request req(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));
      std::string const expectedConnectionKey = AzureSdkHttpbinServer::Host() + "0011";

      {
        // Creating a new connection with default options
        Azure::Core::Http::CurlTransportOptions options;
        auto connection
            = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);

        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);

        auto session = std::make_unique<Azure::Core::Http::CurlSession>(
            req, std::move(connection), options.HttpKeepAlive);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
      }
      // Check that after the connection is gone, it is moved back to the pool
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          1);
      auto connectionFromPool
          = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .begin()
                ->second.begin()
                ->get();
      EXPECT_EQ(connectionFromPool->GetConnectionKey(), expectedConnectionKey);

      // Test that asking a connection with same config will re-use the same connection
      {
        // Creating a new connection with default options
        Azure::Core::Http::CurlTransportOptions options;
        auto connection
            = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);

        // There was just one connection in the pool, it should be empty now
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            0);
        // And the connection key for the connection we got is the expected
        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);

        auto session = std::make_unique<Azure::Core::Http::CurlSession>(
            req, std::move(connection), options.HttpKeepAlive);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
      }
      // Check that after the connection is gone, it is moved back to the pool
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          1);
      auto values = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                        .ConnectionPoolIndex.begin();
      EXPECT_EQ(values->second.size(), 1);
      EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);

      // Now test that using a different connection config won't re-use the same connection
      std::string const secondExpectedKey = AzureSdkHttpbinServer::Host() + "0010";
      {
        // Creating a new connection with options
        Azure::Core::Http::CurlTransportOptions options;
        options.SslVerifyPeer = false;
        auto connection
            = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);
        EXPECT_EQ(connection->GetConnectionKey(), secondExpectedKey);
        // One connection still in the pool after getting a new connection and with first expected
        // key
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .begin()
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
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          2);
      values = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                   .ConnectionPoolIndex.begin();
      EXPECT_EQ(values->second.size(), 1);
      EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), secondExpectedKey);
      values++;
      EXPECT_EQ(values->second.size(), 1);
      EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);

      // Test re-using same custom config
      {
        // Creating a new connection with default options
        Azure::Core::Http::CurlTransportOptions options;
        auto connection
            = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);
        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);
        // One connection still in the pool after getting a new connection and with first expected
        // key
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .begin()
                ->second.begin()
                ->get()
                ->GetConnectionKey(),
            secondExpectedKey);

        auto session = std::make_unique<Azure::Core::Http::CurlSession>(
            req, std::move(connection), options.HttpKeepAlive);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
      }
      // Now there should be 2 index wit one connection each
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          2);
      values = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                   .ConnectionPoolIndex.begin();
      EXPECT_EQ(values->second.size(), 1);
      EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), secondExpectedKey);
      values++;
      EXPECT_EQ(values->second.size(), 1);
      EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);

#ifdef RUN_LONG_UNIT_TESTS
      {
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            2);

        // Test pool clean routine.
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

        // Ensure connections and index is removed
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            0);
      }
#endif
      // Test max connections in pool. Try to add 2k connections to the pool.
      // Using fake connections to avoid opening real http connections :)
      {
        using ::testing::_;
        using ::testing::Return;
        using ::testing::ReturnRef;

        {
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          // clean the pool
          CurlConnectionPool::g_curlConnectionPool.ResetPool();
        }

        std::string hostKey("key");
        for (uint64_t count = 0; count < 2000; count++)
        {
          MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
          EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(hostKey));
          EXPECT_CALL(*curlMock, UpdateLastUsageTime()).WillRepeatedly(Return());
          EXPECT_CALL(*curlMock, IsExpired()).WillRepeatedly(Return(false));
          EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _)).WillRepeatedly(Return(count));
          EXPECT_CALL(*curlMock, DestructObj());

          CurlConnectionPool::g_curlConnectionPool.MoveConnectionBackToPool(
              std::unique_ptr<MockCurlNetworkConnection>(curlMock),
              Azure::Core::Http::HttpStatusCode::Ok);
        }
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                .ConnectionPoolIndex[hostKey]
                .size(),
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                .m_maxConnectionsPerIndex);
        // Test the first and last connection. Each connection should remove the last and oldest
        auto connectionIt = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                                .ConnectionPoolIndex[hostKey]
                                .begin();
        EXPECT_EQ(
            connectionIt->get()->ReadFromSocket(nullptr, 0, Context::GetApplicationContext()),
            2000 - 1); // starting from zero
        connectionIt = --(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                              .ConnectionPoolIndex[hostKey]
                              .end());
        EXPECT_EQ(
            connectionIt->get()->ReadFromSocket(nullptr, 0, Context::GetApplicationContext()),
            2000 - 1024);

        // Check the pool will take other host-key
        {
          std::string otherKey("otherHostKey");
          MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
          EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(otherKey));
          EXPECT_CALL(*curlMock, UpdateLastUsageTime()).WillRepeatedly(Return());
          EXPECT_CALL(*curlMock, IsExpired()).WillRepeatedly(Return(false));
          EXPECT_CALL(*curlMock, DestructObj());

          CurlConnectionPool::g_curlConnectionPool.MoveConnectionBackToPool(
              std::unique_ptr<MockCurlNetworkConnection>(curlMock),
              Azure::Core::Http::HttpStatusCode::Ok);

          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex.size(),
              2);
          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex[otherKey]
                  .size(),
              1);
          // No changes to the full pool
          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex[hostKey]
                  .size(),
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .m_maxConnectionsPerIndex);
        }
        {
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          // clean the pool
          CurlConnectionPool::g_curlConnectionPool.ResetPool();
        }
      }
    }

    TEST(CurlConnectionPool, resiliencyOnConnectionClosed)
    {
      Azure::Core::Http::Request req(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));

      Azure::Core::Http::CurlTransportOptions options;
      auto connection
          = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);
      // Simulate connection lost (like server disconnection).
      connection->Shutdown();

      {
        // Check that CURLE_SEND_ERROR is produced when trying to use the connection.
        auto session = std::make_unique<Azure::Core::Http::CurlSession>(
            req, std::move(connection), options.HttpKeepAlive);
        auto r = session->Perform(Azure::Core::Context::GetApplicationContext());
        EXPECT_EQ(CURLE_SEND_ERROR, r);
      }
    }

#endif
}}} // namespace Azure::Core::Test
