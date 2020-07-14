// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter.hpp"
#include <string>

namespace Azure { namespace Core { namespace Test {

  static std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> CreatePolicies()
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> p;
    std::shared_ptr<Azure::Core::Http::HttpTransport> transport
        = std::make_shared<Azure::Core::Http::CurlTransport>();

    p.push_back(std::make_unique<Azure::Core::Http::TransportPolicy>(std::move(transport)));
    return p;
  }

  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> TransportAdapter::policies
      = CreatePolicies();

  Azure::Core::Http::HttpPipeline TransportAdapter::pipeline(policies);
  Azure::Core::Context TransportAdapter::context = Azure::Core::Context();

  TEST_F(TransportAdapter, get)
  {
    std::string host("http://httpbin.org/get");
    auto expectedResponseBodySize
        = 199; // May fail in the future if Server change the response size.

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto body = response->GetBodyStream();
    EXPECT_EQ(body->Length(), expectedResponseBodySize);

    // Add a header and send again. Response should return that header in the body
    request.AddHeader("123", "456");
    response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    body = response->GetBodyStream();
    EXPECT_EQ(
        body->Length(),
        expectedResponseBodySize + 6
            + 13); // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
  }
}}} // namespace Azure::Core::Test
