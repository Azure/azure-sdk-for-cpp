// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/certificates/certificate_client.hpp"

#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"
#include "private/keyvault_certificates_common_request.hpp"
#include "private/package_version.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Security::KeyVault::Certificates::_detail;
using namespace Azure;
using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;
using namespace Azure::Security::KeyVault::_detail;

namespace {
// This is a Key-Vault only patch to calculate token scope/audience
std::string GetScopeFromUrl(Azure::Core::Url const& url)
{
  std::string calculatedScope(url.GetScheme() + "://");
  auto const& hostWithAccount = url.GetHost();
  auto hostNoAccountStart = std::find(hostWithAccount.begin(), hostWithAccount.end(), '.');

  // Insert the calculated scope only when then host in the url contains at least a `.`
  // Otherwise, only the default scope will be there.
  // We don't want to throw/validate input but just leave the values go to azure to decide what to
  // do.
  if (hostNoAccountStart != hostWithAccount.end())
  {
    calculatedScope.append(hostNoAccountStart + 1, hostWithAccount.end());
    calculatedScope.append("/.default");
  }

  return calculatedScope;
}
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

CertificateClient::CertificateClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CertificateClientOptions options)
    : m_vaultUrl(vaultUrl), m_apiVersion(options.Version.ToString())
{
  auto apiVersion = options.Version.ToString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{::GetScopeFromUrl(m_vaultUrl)}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, std::move(tokenContext)));
  }
  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      KeyVaultServicePackageName,
      PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}

Response<KeyVaultCertificateWithPolicy> CertificateClient::GetCertificate(
    std::string const& name,
    Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value
      = _detail::KeyVaultCertificateSerializer::KeyVaultCertificateDeserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Response<KeyVaultCertificate> CertificateClient::GetCertificateVersion(
    std::string const& name,
    GetCertificateOptions const& options,
    Context const& context) const
{
  // Request with no payload
  std::vector<std::string> path{{CertificatesPath, name}};
  if (!options.Version.empty())
  {
    path.emplace_back(options.Version);
  }

  auto request = CreateRequest(HttpMethod::Get, std::move(path));

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value
      = _detail::KeyVaultCertificateSerializer::KeyVaultCertificateDeserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificate>(std::move(value), std::move(rawResponse));
}

CreateCertificateOperation CertificateClient::StartCreateCertificate(
    std::string const& name,
    CertificateCreateParameters const& parameters,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateCreateParametersSerializer::Serialize(parameters);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Post, {CertificatesPath, name, CertificatesCreatePath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = _detail::CertificateOperationSerializer::Deserialize(*rawResponse);
  auto responseT
      = Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
  return CreateCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}

Azure::Response<CertificateIssuer> CertificateClient::GetIssuer(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, IssuersPath, name});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateIssuerSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::DeleteIssuer(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, IssuersPath, name});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateIssuerSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::CreateIssuer(
    CertificateIssuer const& issuer,
    Azure::Core::Context const& context) const
{
  std::string name = issuer.Name;
  auto payload = CertificateIssuerSerializer::Serialize(issuer);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request
      = CreateRequest(HttpMethod::Put, {CertificatesPath, IssuersPath, name}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = CertificateIssuerSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::UpdateIssuer(
    CertificateIssuer const& issuer,
    Azure::Core::Context const& context) const
{
  std::string name = issuer.Name;
  auto payload = CertificateIssuerSerializer::Serialize(issuer);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request
      = CreateRequest(HttpMethod::Patch, {CertificatesPath, IssuersPath, name}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = CertificateIssuerSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Response<std::vector<CertificateContact>> CertificateClient::GetContacts(
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, ContactsPath});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = CertificateContactsSerializer::Deserialize(*rawResponse);
  return Azure::Response<std::vector<CertificateContact>>(std::move(value), std::move(rawResponse));
}

Response<std::vector<CertificateContact>> CertificateClient::DeleteContacts(
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, ContactsPath});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = CertificateContactsSerializer::Deserialize(*rawResponse);
  return Azure::Response<std::vector<CertificateContact>>(std::move(value), std::move(rawResponse));
}

Response<std::vector<CertificateContact>> CertificateClient::SetContacts(
    std::vector<CertificateContact> const& contacts,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateContactsSerializer::Serialize(contacts);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(HttpMethod::Put, {CertificatesPath, ContactsPath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = CertificateContactsSerializer::Deserialize(*rawResponse);
  return Azure::Response<std::vector<CertificateContact>>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateOperationProperties> CertificateClient::GetCertificateOperation(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, name, PendingPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

CerticatePropertiesPagedResponse CertificateClient::GetPropertiesOfCertificates(
    GetPropertiesOfCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  (void)options;
  (void)context;
  CerticatePropertiesPagedResponse response;
  return response;
}

CerticatePropertiesPagedResponse CertificateClient::GetPropertiesOfCertificateVersions(
    std::string const& name,
    GetPropertiesOfCertificateVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  (void)name;
  (void)options;
  (void)context;
  CerticatePropertiesPagedResponse response;
  return response;
}

const ServiceVersion ServiceVersion::V7_2("7.2");
