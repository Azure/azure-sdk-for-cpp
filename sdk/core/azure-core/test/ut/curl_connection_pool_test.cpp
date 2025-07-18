// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "transport_adapter_base_test.hpp"

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
// They are included to test the connection pool from the libcurl transport adapter implementation.
#include "curl_session_test.hpp"

#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

using testing::ValuesIn;
using namespace Azure::Core::Http::_detail;
using namespace std::chrono_literals;

namespace {
inline std::string CreateConnectionKey(
    std::string const& schema,
    std::string const& host,
    std::string const& configurationKey)
{
  return schema + "://" + host + configurationKey;
}
} // namespace

namespace Azure { namespace Core { namespace Test {

/***********************  Unique Tests for Libcurl   ********************************/
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#if _azure_DISABLE_HTTP_BIN_TESTS
    TEST(CurlConnectionPool, DISABLED_connectionPoolTest)
#else
    TEST(CurlConnectionPool, connectionPoolTest)
#endif
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
      std::string const expectedConnectionKey(CreateConnectionKey(
          AzureSdkHttpbinServer::Schema(),
          AzureSdkHttpbinServer::Host(),
          ",0,0,0,0,0,1,1,0,0,0,1,0,0"));

      {
        // Creating a new connection with default options
        Azure::Core::Http::CurlTransportOptions options;
        auto connection
            = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);

        EXPECT_EQ(connection->GetConnectionKey(), expectedConnectionKey);

        auto session
            = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
        session->m_httpKeepAlive = true;
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

        auto session
            = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
        session->m_httpKeepAlive = true;
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
      std::string const secondExpectedKey = AzureSdkHttpbinServer::Schema() + "://"
          + AzureSdkHttpbinServer::Host() + ",0,0,0,0,0,1,0,0,0,0,1,0,200000";
      {
        // Creating a new connection with options
        Azure::Core::Http::CurlTransportOptions options;
        options.SslVerifyPeer = false;
        options.ConnectionTimeout = std::chrono::seconds(200);
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

        auto session
            = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
        session->m_httpKeepAlive = true;
      }

      // Now there should be 2 index wit one connection each
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          2);
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        for (auto& val : Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                             .ConnectionPoolIndex)
        {
          EXPECT_EQ(val.second.size(), 1);
        }
        // The connection pool should have the two connections we added earlier.
        EXPECT_NE(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .end(),
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .find(expectedConnectionKey));
        EXPECT_NE(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .end(),
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .find(secondExpectedKey));
      }

      {
        Azure::Core::Http::CurlSession::ResponseBufferParser responseParser;
        EXPECT_EQ(responseParser.ExtractResponse(), nullptr);

        const uint8_t responseBuf[] = "HTTP/1.1 200 OK\r\n\r\n";
        static_cast<void>(responseParser.Parse(responseBuf, sizeof(responseBuf) - 1));
        EXPECT_NE(responseParser.ExtractResponse(), nullptr);
        EXPECT_EQ(responseParser.ExtractResponse(), nullptr);
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

        auto session
            = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);
        // Simulate connection was used already
        session->m_lastStatusCode = Azure::Core::Http::HttpStatusCode::Ok;
        session->m_sessionState = Azure::Core::Http::CurlSession::SessionState::STREAMING;
        session->m_httpKeepAlive = true;
      }
      // Now there should be 2 index wit one connection each
      EXPECT_EQ(
          Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
              .size(),
          2);
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        for (auto& val : Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
                             .ConnectionPoolIndex)
        {
          EXPECT_EQ(val.second.size(), 1);
        }
        // The connection pool should have the two connections we added earlier.
        EXPECT_NE(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .end(),
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .find(expectedConnectionKey));
        EXPECT_NE(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .end(),
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .find(secondExpectedKey));
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
              std::move(connections[count]), true);
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

        auto timeOut = Context{std::chrono::system_clock::now() + 5min};
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
      // Using fake connections to avoid opening real HTTP connections :)
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
      //           true);
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
      //         connectionIt->get()->ReadFromSocket(nullptr, 0, Context{}),
      //         2000 - 1); // starting from zero
      //     connectionIt = --(Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool
      //                           .ConnectionPoolIndex[hostKey]
      //                           .end());
      //     EXPECT_EQ(
      //         connectionIt->get()->ReadFromSocket(nullptr, 0, Context{}),
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
      //           true);

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
        std::string const expectedConnectionKey(CreateConnectionKey(
            AzureSdkHttpbinServer::Schema(),
            AzureSdkHttpbinServer::Host(),
            ",0,0,0,0,0,1,1,0,0,0,1,0,0"));

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
            .MoveConnectionBackToPool(std::move(connection), true);
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
        std::string const authority(AzureSdkHttpbinServer::GetWithPort());
        Azure::Core::Http::Request req(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(authority));
        std::string const expectedConnectionKey(CreateConnectionKey(
            AzureSdkHttpbinServer::Schema(),
            AzureSdkHttpbinServer::Host(),
            ":443,0,0,0,0,0,1,1,0,0,0,1,0,0"));

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
            .MoveConnectionBackToPool(std::move(connection), true);
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
        std::string const expectedConnectionKey(CreateConnectionKey(
            AzureSdkHttpbinServer::Schema(),
            AzureSdkHttpbinServer::Host(),
            ",0,0,0,0,0,1,1,0,0,0,1,0,0"));

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
            .MoveConnectionBackToPool(std::move(connection), true);
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
        std::string const authority(AzureSdkHttpbinServer::GetWithPort());
        Azure::Core::Http::Request req(
            Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(authority));
        std::string const expectedConnectionKey(CreateConnectionKey(
            AzureSdkHttpbinServer::Schema(),
            AzureSdkHttpbinServer::Host(),
            ":443,0,0,0,0,0,1,1,0,0,0,1,0,0"));

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
            .MoveConnectionBackToPool(std::move(connection), true);
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
        auto session
            = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);
        auto r = session->Perform(Azure::Core::Context{});
        EXPECT_EQ(CURLE_SEND_ERROR, r);
      }
    }

#if _azure_DISABLE_HTTP_BIN_TESTS
    TEST(CurlConnectionPool, DISABLED_forceConnectionClosed)
#else
    TEST(CurlConnectionPool, forceConnectionClosed)
#endif
    {
      Azure::Core::Http::Request req(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Status(101)));

      Azure::Core::Http::CurlTransportOptions options;
      auto connection
          = CurlConnectionPool::g_curlConnectionPool.ExtractOrCreateCurlConnection(req, options);

      {
        // Check we can use the connection to retrieve headers in the response, but the connection
        // is still flagged as shutdown.
        auto session
            = std::make_unique<Azure::Core::Http::CurlSession>(req, std::move(connection), options);

        auto r = session->Perform(Azure::Core::Context{});
        EXPECT_EQ(CURLE_OK, r);
        auto response = session->ExtractResponse();
        EXPECT_EQ(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::SwitchingProtocols);
        EXPECT_EQ("close", response->GetHeaders().find("Connection")->second);
      }
    }

#if _azure_DISABLE_HTTP_BIN_TESTS
    TEST(CurlConnectionPool, DISABLED_connectionClose)
#else
    TEST(CurlConnectionPool, connectionClose)
#endif
    {
      /// When getting the header connection: close from an HTTP response, the connection should not
      /// be moved back to the pool.
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.clear();
        // Make sure there are nothing in the pool
        EXPECT_EQ(CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex.size(), 0);
      }

      // Use the same request for all connections.
      Azure::Core::Http::Request req(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(AzureSdkHttpbinServer::Headers()));
      // Server will return this header back
      req.SetHeader("connection", "close");

      {
        // Create a pipeline to send the request and dispose after it.
        Azure::Core::Http::_internal::HttpPipeline pipeline({}, "test", "test", {}, {});
        auto response = pipeline.Send(req, Azure::Core::Context{});
        EXPECT_PRED2(
            [](Azure::Core::Http::HttpStatusCode a, Azure::Core::Http::HttpStatusCode b) {
              return a == b;
            },
            response->GetStatusCode(),
            Azure::Core::Http::HttpStatusCode::Ok);
      }

      // Check that after the connection is gone, it is moved back to the pool
      {
        std::lock_guard<std::mutex> lock(
            CurlConnectionPool::g_curlConnectionPool.ConnectionPoolMutex);
        EXPECT_EQ(
            Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
                .size(),
            0);
      }
    }
#endif
}}} // namespace Azure::Core::Test
