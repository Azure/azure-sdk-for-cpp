// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "curl_session.hpp"

#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
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
            SetArrayArgument<1>(response.data(), response.data() + response.size()),
            Return(response.size())));

    // Create the unique ptr to take care about memory free at the end
    std::unique_ptr<MockCurlNetworkConnection> uniqueCurlMock(curlMock);

    // Simulate a request to be sent
    Azure::Core::Http::Url url("http://microsoft.com");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

    // Move the curlMock to build a session and then send the request
    // The session will get the response we mock before, so it will pass for this GET
    auto session
        = std::make_unique<Azure::Core::Http::CurlSession>(request, std::move(uniqueCurlMock));

    EXPECT_NO_THROW(session->Perform(Azure::Core::GetApplicationContext()));
  }
}}} // namespace Azure::Core::Test
