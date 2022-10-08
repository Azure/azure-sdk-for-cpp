// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "curl_session_test.hpp"

#include <azure/core/http/curl_transport.hpp>
#include <azure/core/http/http.hpp>

#include <http/curl/curl_connection_private.hpp>
#include <http/curl/curl_session_private.hpp>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArrayArgument;

namespace Azure { namespace Core { namespace Test {

  TEST_F(CurlSession, successCall)
  {
    std::string response(
        "HTTP/1.1 200 Ok\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n{\r\n\"somejson\":45\r}");
    int32_t const payloadSize = static_cast<int32_t>(response.size());

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + payloadSize),
            Return(payloadSize)));

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    // Move the curlMock to build a session and then send the request
    // The session will get the response we mock before, so it will pass for this GET
    Azure::Core::Http::CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = true;
    auto session = std::make_unique<Azure::Core::Http::CurlSession>(
        request, std::move(uniqueCurlMock), transportOptions);

    EXPECT_NO_THROW(session->Perform(Azure::Core::Context::ApplicationContext));
  }

  TEST_F(CurlSession, chunkResponseSizeZero)
  {
    // chunked response with no content and no size
    std::string response("HTTP/1.1 200 Ok\r\ntransfer-encoding: chunked\r\n\r\n\n\r\n");
    std::string connectionKey("connection-key");
    int32_t const payloadSize = static_cast<int32_t>(response.size());

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + payloadSize),
            Return(payloadSize)));
    EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(connectionKey));
    EXPECT_CALL(*curlMock, UpdateLastUsageTime());
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      Azure::Core::Http::CurlTransportOptions transportOptions;
      transportOptions.HttpKeepAlive = true;

      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), transportOptions);

      EXPECT_NO_THROW(session->Perform(Azure::Core::Context::ApplicationContext));
    }
    // Clear the connections from the pool to invoke clean routine
    Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
        .clear();
  }

  TEST_F(CurlSession, chunkBadFormatResponse)
  {
    // chunked response with unexpected char at the end
    std::string response("HTTP/1.1 200 Ok\r\ntransfer-encoding: chunked\r\n\r\n9\r\n");
    std::string response2("123456789\r\n0\r\n\rx");
    std::string connectionKey("connection-key");
    int32_t const payloadSize = static_cast<int32_t>(response.size());
    int32_t const payloadSize2 = static_cast<int32_t>(response2.size());

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + payloadSize),
            Return(payloadSize)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response2.data(), response2.data() + payloadSize2),
            Return(payloadSize2)));
    EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(connectionKey));
    EXPECT_CALL(*curlMock, UpdateLastUsageTime());
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      Azure::Core::Http::CurlTransportOptions transportOptions;
      transportOptions.HttpKeepAlive = true;
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), transportOptions);

      EXPECT_NO_THROW(session->Perform(Azure::Core::Context::ApplicationContext));
      auto r = session->ExtractResponse();
      r->SetBodyStream(std::move(session));
      auto bodyS = r->ExtractBodyStream();

      // Read the bodyStream to get all chunks
      EXPECT_THROW(
          bodyS->ReadToEnd(Azure::Core::Context::ApplicationContext),
          Azure::Core::Http::TransportException);
    }
    // Clear the connections from the pool to invoke clean routine
    Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
        .clear();
  }

  TEST_F(CurlSession, invalidHeader)
  {
    std::string response("HTTP/1.1 200 Ok\r\ninvalid header\r\n\r\nbody");
    int32_t const payloadSize = static_cast<int32_t>(response.size());

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + payloadSize),
            Return(payloadSize)));

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    // Move the curlMock to build a session and then send the request
    // The session will get the response we mock before, so it will pass for this GET
    Azure::Core::Http::CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = true;
    auto session = std::make_unique<Azure::Core::Http::CurlSession>(
        request, std::move(uniqueCurlMock), transportOptions);

    EXPECT_THROW(session->Perform(Azure::Core::Context::ApplicationContext), std::invalid_argument);
  }

  TEST_F(CurlSession, emptyHeaderValue)
  {
    std::string response("HTTP/1.1 200 Ok\r\nheader:\r\n\r\nbody");
    int32_t const payloadSize = static_cast<int32_t>(response.size());

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + payloadSize),
            Return(payloadSize)));

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    // Move the curlMock to build a session and then send the request
    // The session will get the response we mock before, so it will pass for this GET
    Azure::Core::Http::CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = true;
    auto session = std::make_unique<Azure::Core::Http::CurlSession>(
        request, std::move(uniqueCurlMock), transportOptions);

    EXPECT_NO_THROW(session->Perform(Azure::Core::Context::ApplicationContext));
  }

  TEST_F(CurlSession, headerValueWhitespace)
  {
    std::string response("HTTP/1.1 200 Ok\r\nheader: \tvalue\r\n\r\nbody");
    int32_t const payloadSize = static_cast<int32_t>(response.size());

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + payloadSize),
            Return(payloadSize)));

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    // Move the curlMock to build a session and then send the request
    // The session will get the response we mock before, so it will pass for this GET
    Azure::Core::Http::CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = true;
    auto session = std::make_unique<Azure::Core::Http::CurlSession>(
        request, std::move(uniqueCurlMock), transportOptions);

    EXPECT_NO_THROW(session->Perform(Azure::Core::Context::ApplicationContext));
  }

  TEST_F(CurlSession, chunkSegmentedResponse)
  {
    // chunked response - simulate the data that the wire will return on every read
    std::string response0("HTTP/1.1 200 Ok\r");
    std::string response1("\ntransfer-encoding:");
    std::string response2(" chunke"); // cspell:disable-line
    std::string response3("d\r\n");
    std::string response4("\r");
    std::string response5("\n3\r\n");
    std::string response6("123");
    std::string response7("\r\n0\r\n");
    std::string response8("\r\n");
    int32_t const payloadSize0 = static_cast<int32_t>(response0.size());
    int32_t const payloadSize1 = static_cast<int32_t>(response1.size());
    int32_t const payloadSize2 = static_cast<int32_t>(response2.size());
    int32_t const payloadSize3 = static_cast<int32_t>(response3.size());
    int32_t const payloadSize4 = static_cast<int32_t>(response4.size());
    int32_t const payloadSize5 = static_cast<int32_t>(response5.size());
    int32_t const payloadSize6 = static_cast<int32_t>(response6.size());
    int32_t const payloadSize7 = static_cast<int32_t>(response7.size());
    int32_t const payloadSize8 = static_cast<int32_t>(response8.size());

    std::string connectionKey("connection-key");

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response0.data(), response0.data() + payloadSize0),
            Return(payloadSize0)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response1.data(), response1.data() + payloadSize1),
            Return(payloadSize1)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response2.data(), response2.data() + payloadSize2),
            Return(payloadSize2)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response3.data(), response3.data() + payloadSize3),
            Return(payloadSize3)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response4.data(), response4.data() + payloadSize4),
            Return(payloadSize4)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response5.data(), response5.data() + payloadSize5),
            Return(payloadSize5)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response6.data(), response6.data() + payloadSize6),
            Return(payloadSize6)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response7.data(), response7.data() + payloadSize7),
            Return(payloadSize7)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response8.data(), response8.data() + payloadSize8),
            Return(payloadSize8)));
    EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(connectionKey));
    EXPECT_CALL(*curlMock, UpdateLastUsageTime());
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      Azure::Core::Http::CurlTransportOptions transportOptions;
      transportOptions.HttpKeepAlive = true;
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), transportOptions);

      EXPECT_NO_THROW(session->Perform(Azure::Core::Context::ApplicationContext));
      auto response = session->ExtractResponse();
      response->SetBodyStream(std::move(session));
      auto bodyS = response->ExtractBodyStream();

      // Read the bodyStream to get all chunks
      EXPECT_NO_THROW(bodyS->ReadToEnd(Azure::Core::Context::ApplicationContext));
    }
    // Clear the connections from the pool to invoke clean routine
    Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
        .clear();
  }

  TEST_F(CurlSession, DoNotReuseConnectionIfDownloadFail)
  {
    Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
        .clear();
    // Can't mock the curlMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    // mock an upload error
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_SEND_ERROR));
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      Azure::Core::Http::CurlTransportOptions transportOptions;
      transportOptions.HttpKeepAlive = true;
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), transportOptions);

      auto returnCode = session->Perform(Azure::Core::Context::ApplicationContext);
      EXPECT_EQ(CURLE_SEND_ERROR, returnCode);
    }
    // Check connection pool is empty (connection was not moved to the pool)
    EXPECT_EQ(
        Azure::Core::Http::_detail::CurlConnectionPool::g_curlConnectionPool.ConnectionPoolIndex
            .size(),
        0);
  }
}}} // namespace Azure::Core::Test
