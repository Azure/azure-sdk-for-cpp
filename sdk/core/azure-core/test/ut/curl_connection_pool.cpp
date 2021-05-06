// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

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
// That's why the path starts from `private/`
// They are included to test the connection pool from the curl transport adapter implementation.
#include <private/curl_connection.hpp>
#include <private/curl_connection_pool.hpp>
#include <private/curl_session.hpp>

#include "curl_session.hpp"

using testing::ValuesIn;
using namespace Azure::Core::Http::_detail;
using namespace std::chrono_literals;

namespace Azure { namespace Core { namespace Test {

/***********************  Unique Tests for Libcurl   ********************************/
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)

    TEST(CurlConnectionPool, connectionPoolTest)
    {
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.clear();
        // Make sure there are nothing in the pool
        EXPECT_EQ(CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.size(), 0);
      }

      // Use the same request for all connections.
      Azure::Core::Http::Request req(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Get()));
      std::string const expectedConnectionKey
          = AzureSdkHttpbinServer::Schema() + AzureSdkHttpbinServer::Host() + "0011";

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
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
        auto connectionFromPool
            = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex.begin()
                  ->second.begin()
                  ->get();
        EXPECT_EQ(connectionFromPool->GetConnectionKey(), expectedConnectionKey);
      }

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
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        // Check that after the connection is gone, it is moved back to the pool
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
        auto values = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                          .ConnectionPoolIndex.begin();
        EXPECT_EQ(values->second.size(), 1);
        EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);
      }

      // Now test that using a different connection config won't re-use the same connection
      std::string const secondExpectedKey
          = AzureSdkHttpbinServer::Schema() + AzureSdkHttpbinServer::Host() + "0010";
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
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        auto values = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                          .ConnectionPoolIndex.begin();
        EXPECT_EQ(values->second.size(), 1);
        EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), secondExpectedKey);
        values++;
        EXPECT_EQ(values->second.size(), 1);
        EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);
      }

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
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        auto values = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                          .ConnectionPoolIndex.begin();
        EXPECT_EQ(values->second.size(), 1);
        EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), secondExpectedKey);
        values++;
        EXPECT_EQ(values->second.size(), 1);
        EXPECT_EQ(values->second.begin()->get()->GetConnectionKey(), expectedConnectionKey);
      }
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        // clean the pool
        CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.clear();
      }

#ifdef RUN_LONG_UNIT_TESTS
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        // clean the pool
        CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.clear();
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            0);
      }

      // Test pool clean routine.
      std::cout << "Running Connection Pool Cleaner Test. This test can take up to 2 minutes to "
                   "complete."
                << std::endl
                << "Add compiler option -DRUN_LONG_UNIT_TESTS=OFF when building if you want to "
                   "skip this test."
                << std::endl;
      {
        // Make sure the clean pool thread is started by adding 5 connections to the pool
        std::vector<std::unique_ptr<Azure::Core::Http::CurlNetworkConnection>> connections;
        for (int count = 0; count < 5; count++)
        {
          connections.emplace_back(
              CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, {}));
        }
        for (int count = 0; count < 5; count++)
        {
          CurlConnectionPool::g_curlConnectionPool.MoveConnectionBackToPool(
              std::move(connections[count]), Http::HttpStatusCode::Ok);
        }
      }

      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                .ConnectionPoolIndex[expectedConnectionKey]
                .size(),
            5);
      }

      // Wait for 60 secs (default time to expire a connection)
      std::this_thread::sleep_for(60ms);

      {
        // Now check the pool until the clean thread until finishes removing the connections or
        // fail after 5 minutes (indicates a problem with the clean routine)

        auto timeOut
            = Context::ApplicationContext.WithDeadline(std::chrono::system_clock::now() + 5min);
        bool poolIsEmpty = false;
        while (!poolIsEmpty && !timeOut.IsCancelled())
        {
          std::this_thread::sleep_for(10ms);
          // If test wakes while clean pool is running, it will wait until lock is released by
          // the clean pool thread.
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          poolIsEmpty = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                            .ConnectionPoolIndex.size()
              == 0;
        }
        EXPECT_TRUE(poolIsEmpty);
      }

#endif
      // Test max connections in pool. Try to add 2k connections to the pool.
      // Using fake connections to avoid opening real http connections :)
      //   {
      //     using ::testing::_;
      //     using ::testing::Return;
      //     using ::testing::ReturnRef;

      //     {
      //       std::lock_guard<std::mutex> lock(
      //           CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
      //       // clean the pool
      //       CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.clear();
      //     }

      //     std::string hostKey("key");
      //     for (uint64_t count = 0; count < 2000; count++)
      //     {
      //       MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
      //       EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(hostKey));
      //       EXPECT_CALL(*curlMock, UpdateLastUsageTime()).WillRepeatedly(Return());
      //       EXPECT_CALL(*curlMock, IsExpired()).WillRepeatedly(Return(false));
      //       EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _)).WillRepeatedly(Return(count));
      //       EXPECT_CALL(*curlMock, DestructObj());

      //       CurlConnectionPool::g_curlConnectionPool.MoveConnectionBackToPool(
      //           std::unique_ptr<MockCurlNetworkConnection>(curlMock),
      //           Azure::Core::Http::HttpStatusCode::Ok);
      //     }
      //     // No need to take look here because connections are mocked to never be expired.
      //     EXPECT_EQ(
      //         Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
      //             .size(),
      //         1);
      //     EXPECT_EQ(
      //         Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //             .ConnectionPoolIndex[hostKey]
      //             .size(),
      //         Azure::Core::Http::_detail::MaxConnectionsPerIndex);
      //     // Test the first and last connection. Each connection should remove the last and
      //     oldest auto connectionIt =
      //     Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //                             .ConnectionPoolIndex[hostKey]
      //                             .begin();
      //     EXPECT_EQ(
      //         connectionIt->get()->ReadFromSocket(nullptr, 0, Context::ApplicationContext),
      //         2000 - 1); // starting from zero
      //     connectionIt = --(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //                           .ConnectionPoolIndex[hostKey]
      //                           .end());
      //     EXPECT_EQ(
      //         connectionIt->get()->ReadFromSocket(nullptr, 0, Context::ApplicationContext),
      //         2000 - 1024);

      //     // Check the pool will take other host-key
      //     {
      //       std::string otherKey("otherHostKey");
      //       MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
      //       EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(otherKey));
      //       EXPECT_CALL(*curlMock, UpdateLastUsageTime()).WillRepeatedly(Return());
      //       EXPECT_CALL(*curlMock, IsExpired()).WillRepeatedly(Return(false));
      //       EXPECT_CALL(*curlMock, DestructObj());

      //       CurlConnectionPool::g_curlConnectionPool.MoveConnectionBackToPool(
      //           std::unique_ptr<MockCurlNetworkConnection>(curlMock),
      //           Azure::Core::Http::HttpStatusCode::Ok);

      //       EXPECT_EQ(
      //           Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //               .ConnectionPoolIndex.size(),
      //           2);
      //       EXPECT_EQ(
      //           Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //               .ConnectionPoolIndex[otherKey]
      //               .size(),
      //           1);
      //       // No changes to the full pool
      //       EXPECT_EQ(
      //           Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //               .ConnectionPoolIndex[hostKey]
      //               .size(),
      //           Azure::Core::Http::_detail::MaxConnectionsPerIndex);
      //     }
      //     {
      //       std::lock_guard<std::mutex> lock(
      //           CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
      //       // clean the pool
      //       CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.clear();
      //     }
      //   }
    }

    TEST(CurlConnectionPool, uniquePort)
    {
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
            .clear();
        // Make sure there is nothing in the pool
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            0);
      }

      {
        // Request with no port
        std::string const authority(AzureSdkHttpbinServer::Get());
        Azure::Core::Http::Request req(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(authority));
        std::string const expectedConnectionKey
            = AzureSdkHttpbinServer::Schema() + AzureSdkHttpbinServer::Host() + "0011";

        // Creating a new connection with default options
        auto connection = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                              .ExtractOrCreateCurlConnection(req, {});

        {
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex.size(),
              0);
          EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);
        }
        // move connection back to the pool
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
            .MoveConnectionBackToPool(std::move(connection), Azure::Core::Http::HttpStatusCode::Ok);
      }

      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        // Test connection was moved to the pool
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            1);
      }

      {
        // Request with port
        std::string const authority(AzureSdkHttpbinServer::WithPort());
        Azure::Core::Http::Request req(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(authority));
        std::string const expectedConnectionKey
            = AzureSdkHttpbinServer::Schema() + AzureSdkHttpbinServer::Host() + "4430011";

        // Creating a new connection with default options
        auto connection = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                              .ExtractOrCreateCurlConnection(req, {});

        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);
        {
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          // Check connection in pool is not re-used because the port is different
          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex.size(),
              1);
        }
        // move connection back to the pool
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
            .MoveConnectionBackToPool(std::move(connection), Azure::Core::Http::HttpStatusCode::Ok);
      }
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        // Check 2 connections in the pool
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            2);
      }

      // Re-use connections
      {
        // Request with no port
        std::string const authority(AzureSdkHttpbinServer::Get());
        Azure::Core::Http::Request req(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(authority));
        std::string const expectedConnectionKey
            = AzureSdkHttpbinServer::Schema() + AzureSdkHttpbinServer::Host() + "0011";

        // Creating a new connection with default options
        auto connection = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                              .ExtractOrCreateCurlConnection(req, {});

        {
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex.size(),
              1);
        }
        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);
        // move connection back to the pool
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
            .MoveConnectionBackToPool(std::move(connection), Azure::Core::Http::HttpStatusCode::Ok);
      }

      {
        // Make sure there is nothing in the pool
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            2);
      }
      {
        // Request with port
        std::string const authority(AzureSdkHttpbinServer::WithPort());
        Azure::Core::Http::Request req(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(authority));
        std::string const expectedConnectionKey
            = AzureSdkHttpbinServer::Schema() + AzureSdkHttpbinServer::Host() + "4430011";

        // Creating a new connection with default options
        auto connection = Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                              .ExtractOrCreateCurlConnection(req, {});

        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);
        {
          std::lock_guard<std::mutex> lock(
              CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
          // Check connection in pool is not re-used because the port is different
          EXPECT_EQ(
              Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                  .ConnectionPoolIndex.size(),
              1);
        }
        // move connection back to the pool
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
            .MoveConnectionBackToPool(std::move(connection), Azure::Core::Http::HttpStatusCode::Ok);
      }
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            2);
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
            .clear();
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
        auto r = session->Perform(Azure::Core::Context::ApplicationContext);
        EXPECT_EQ(CURLE_SEND_ERROR, r);
      }
    }

#endif
}}} // namespace Azure::Core::Test
