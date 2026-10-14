// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/attestation/attestation_client.hpp"

#include <azure/core/http/raw_response.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/io/body_stream.hpp>

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace Test {

  // Fake transport which lets the attestation APIs be exercised without a live service.
  class MockAttestationTransport final : public Azure::Core::Http::HttpTransport {
  public:
    std::string LastRequestUrl;
    std::string AttestResponseBody;
    Azure::Core::Http::HttpStatusCode AttestStatusCode = Azure::Core::Http::HttpStatusCode::Ok;

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const&) override
    {
      std::string body;
      auto statusCode = Azure::Core::Http::HttpStatusCode::Ok;
      if (request.GetMethod() == Azure::Core::Http::HttpMethod::Get)
      {
        body = R"({"keys": []})";
      }
      else
      {
        LastRequestUrl = request.GetUrl().GetAbsoluteUrl();
        body = AttestResponseBody;
        statusCode = AttestStatusCode;
      }
      auto response = std::make_unique<Azure::Core::Http::RawResponse>(1, 1, statusCode, "Mock");
      m_bodies.emplace_back(body.begin(), body.end());
      response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(m_bodies.back()));
      return response;
    }

  private:
    // Body buffers must outlive the MemoryBodyStream which references them.
    std::list<std::vector<uint8_t>> m_bodies;
  };

  inline Azure::Security::Attestation::AttestationClient CreateMockClient(
      std::shared_ptr<MockAttestationTransport> transport,
      std::string const& apiVersion = {})
  {
    Azure::Security::Attestation::AttestationClientOptions options;
    options.Transport.Transport = transport;
    options.Retry.MaxRetries = 0;
    if (!apiVersion.empty())
    {
      options.ApiVersion = apiVersion;
    }
    return Azure::Security::Attestation::AttestationClient::Create(
        "https://mock.attest.azure.net", options);
  }

}}}} // namespace Azure::Security::Attestation::Test
