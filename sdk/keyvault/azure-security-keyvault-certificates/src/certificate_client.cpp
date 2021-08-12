// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/certificates/certificate_client.hpp"

#include "private/certificate_serializers.hpp"
#include "private/keyvault_certificates_common_request.hpp"

using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure;
using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Security::KeyVault::_detail;

namespace {
constexpr static const char CertificatesPath[] = "certificates";
constexpr static const char CreateValue[] = "create";
} // namespace

std::unique_ptr<RawResponse> CertificateClient::SendRequest(
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context) const
{
  return KeyVaultCertificatesCommonRequest::SendRequest(*m_pipeline, request, context);
}

Request CertificateClient::CreateRequest(
    HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content) const
{
  return KeyVaultCertificatesCommonRequest::CreateRequest(
      m_vaultUrl, m_apiVersion, method, path, content);
}

Response<KeyVaultCertificateWithPolicy> CertificateClient::GetCertificate(
    std::string const& name,
    Context const& context) const
{
  // Request with no payload
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value
      = _detail::KeyVaultCertificateSerializer::KeyVaultCertificateDeserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}
