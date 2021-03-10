// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "curl_session.hpp"

#include <azure/core/http/curl/curl.hpp>
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

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + response.size()),
            Return(response.size())));

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    // Move the curlMock to build a session and then send the request
    // The session will get the response we mock before, so it will pass for this GET
    auto session = std::make_unique<Azure::Core::Http::CurlSession>(
        request, std::move(uniqueCurlMock), true);

    EXPECT_NO_THROW(session->Perform(Azure::Core::GetApplicationContext()));
  }

  TEST_F(CurlSession, chunkResponseSizeZero)
  {
    // chunked response with no content and no size
    std::string response("HTTP/1.1 200 Ok\r\ntransfer-encoding: chunked\r\n\r\n\n\r\n");
    std::string connectionKey("connection-key");

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + response.size()),
            Return(response.size())));
    EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(connectionKey));
    EXPECT_CALL(*curlMock, updateLastUsageTime());
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), true);

      EXPECT_NO_THROW(session->Perform(Azure::Core::GetApplicationContext()));
    }
    // Clear the connections from the pool to invoke clean routine
    Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.clear();
  }

  TEST_F(CurlSession, chunkBadFormatResponse)
  {
    // chunked response with unexpected char at the end
    std::string response("HTTP/1.1 200 Ok\r\ntransfer-encoding: chunked\r\n\r\n9\r\n");
    std::string response2("123456789\r\n0\r\n\rx");
    std::string connectionKey("connection-key");

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response.data(), response.data() + response.size()),
            Return(response.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response2.data(), response2.data() + response2.size()),
            Return(response2.size())));
    EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(connectionKey));
    EXPECT_CALL(*curlMock, updateLastUsageTime());
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), true);

      EXPECT_NO_THROW(session->Perform(Azure::Core::GetApplicationContext()));
      auto r = session->GetResponse();
      r->SetBodyStream(std::move(session));
      auto bodyS = r->GetBodyStream();

      // Read the bodyStream to get all chunks
      EXPECT_THROW(
          Azure::Core::IO::BodyStream::ReadToEnd(*bodyS, Azure::Core::GetApplicationContext()),
          Azure::Core::Http::TransportException);
    }
    // Clear the connections from the pool to invoke clean routine
    Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.clear();
  }

  TEST_F(CurlSession, chunkSegmentedResponse)
  {
    // chunked response - simulate the data that the wire will return on every read
    std::string response0("HTTP/1.1 200 Ok\r");
    std::string response1("\ntransfer-encoding:");
    std::string response2(" chunke");
    std::string response3("d\r\n");
    std::string response4("\r");
    std::string response5("\n3\r\n");
    std::string response6("123");
    std::string response7("\r\n0\r\n");
    std::string response8("\r\n");

    std::string connectionKey("connection-key");

    // Can't mock the curMock directly from a unique ptr, heap allocate it first and then make a
    // unique ptr for it
    MockCurlNetworkConnection* curlMock = new MockCurlNetworkConnection();
    EXPECT_CALL(*curlMock, SendBuffer(_, _, _)).WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*curlMock, ReadFromSocket(_, _, _))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response0.data(), response0.data() + response0.size()),
            Return(response0.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response1.data(), response1.data() + response1.size()),
            Return(response1.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response2.data(), response2.data() + response2.size()),
            Return(response2.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response3.data(), response3.data() + response3.size()),
            Return(response3.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response4.data(), response4.data() + response4.size()),
            Return(response4.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response5.data(), response5.data() + response5.size()),
            Return(response5.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response6.data(), response6.data() + response6.size()),
            Return(response6.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response7.data(), response7.data() + response7.size()),
            Return(response7.size())))
        .WillOnce(DoAll(
            SetArrayArgument<0>(response8.data(), response8.data() + response8.size()),
            Return(response8.size())));
    EXPECT_CALL(*curlMock, GetConnectionKey()).WillRepeatedly(ReturnRef(connectionKey));
    EXPECT_CALL(*curlMock, updateLastUsageTime());
    EXPECT_CALL(*curlMock, DestructObj());

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    {
      // Create the session inside scope so it is released and the connection is moved to the pool
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), true);

      EXPECT_NO_THROW(session->Perform(Azure::Core::GetApplicationContext()));
      auto response = session->GetResponse();
      response->SetBodyStream(std::move(session));
      auto bodyS = response->GetBodyStream();

      // Read the bodyStream to get all chunks
      EXPECT_NO_THROW(
          Azure::Core::IO::BodyStream::ReadToEnd(*bodyS, Azure::Core::GetApplicationContext()));
    }
    // Clear the connections from the pool to invoke clean routine
    Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.clear();
  }

  TEST_F(CurlSession, DoNotReuseConnectionIfDownloadFail)
  {

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
      auto session = std::make_unique<Azure::Core::Http::CurlSession>(
          request, std::move(uniqueCurlMock), true);

      auto returnCode = session->Perform(Azure::Core::GetApplicationContext());
      EXPECT_EQ(CURLE_SEND_ERROR, returnCode);
    }
    // Check connection pool is empty (connection was not moved to the pool)
    EXPECT_EQ(Azure::Core::Http::CurlConnectionPool::ConnectionPoolIndex.size(), 0);
  }
}}} // namespace Azure::Core::Test
