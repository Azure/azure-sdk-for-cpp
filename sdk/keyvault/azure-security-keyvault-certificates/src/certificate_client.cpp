// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/certificates/certificate_client.hpp"

#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"
#include "private/keyvault_certificates_common_request.hpp"
#include "private/package_version.hpp"
#include <azure/core/base64.hpp>
#include <azure/keyvault/shared/keyvault_shared.hpp>

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

Request CertificateClient::ContinuationTokenRequest(
    std::vector<std::string> const& path,
    const Azure::Nullable<std::string>& NextPageToken) const
{
  if (NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(NextPageToken.Value());
    return Request(HttpMethod::Get, nextPageUrl);
  }
  return CreateRequest(HttpMethod::Get, path);
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
        = {{_internal::UrlScope::GetScopeFromUrl(m_vaultUrl)}};

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
  auto value = _detail::KeyVaultCertificateSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Response<KeyVaultCertificateWithPolicy> CertificateClient::GetCertificateVersion(
    std::string const& name,
    std::string const& version,
    Context const& context) const
{
  // Request with no payload
  std::vector<std::string> path{{CertificatesPath, name, version}};

  auto request = CreateRequest(HttpMethod::Get, std::move(path));

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultCertificateSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
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

Response<DeletedCertificate> CertificateClient::GetDeletedCertificate(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {DeletedCertificatesPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = DeletedCertificateSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<DeletedCertificate>(std::move(value), std::move(rawResponse));
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
    std::string const& name,
    CertificateIssuer const& issuer,
    Azure::Core::Context const& context) const
{
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

Response<CertificateContactsResult> CertificateClient::GetContacts(
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, ContactsPath});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = CertificateContactsSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateContactsResult>(std::move(value), std::move(rawResponse));
}

Response<CertificateContactsResult> CertificateClient::DeleteContacts(
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, ContactsPath});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = CertificateContactsSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateContactsResult>(std::move(value), std::move(rawResponse));
}

Response<CertificateContactsResult> CertificateClient::SetContacts(
    std::vector<CertificateContact> const& contacts,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateContactsSerializer::Serialize(contacts);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(HttpMethod::Put, {CertificatesPath, ContactsPath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = CertificateContactsSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateContactsResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateOperationProperties> CertificateClient::GetPendingCertificateOperation(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, name, PendingPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateOperationProperties>
CertificateClient::CancelPendingCertificateOperation(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  CertificateOperationUpdateParameter parameter;
  parameter.CancelationRequested = true;
  auto payload = CertificateOperationUpdateParameterSerializer::Serialize(parameter);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request
      = CreateRequest(HttpMethod::Patch, {CertificatesPath, name, PendingPath}, &payloadStream);
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateOperationProperties>
CertificateClient::DeletePendingCertificateOperation(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, name, PendingPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

Response<PurgedCertificate> CertificateClient::PurgeDeletedCertificate(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {DeletedCertificatesPath, name});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  PurgedCertificate value;
  return Azure::Response<PurgedCertificate>(std::move(value), std::move(rawResponse));
}

DeleteCertificateOperation CertificateClient::StartDeleteCertificate(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, name});

  auto rawResponse = SendRequest(request, context);
  auto value = DeletedCertificate();
  value.Properties.Name = name;
  auto responseT = Azure::Response<DeletedCertificate>(std::move(value), std::move(rawResponse));
  return DeleteCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}

RecoverDeletedCertificateOperation CertificateClient::StartRecoverDeletedCertificate(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Post, {DeletedCertificatesPath, name, RecoverPath});

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateWithPolicy();
  value.Properties.Name = name;
  auto responseT
      = Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
  return RecoverDeletedCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}
Azure::Response<CertificatePolicy> CertificateClient::GetCertificatePolicy(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, name, PolicyPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificatePolicySerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificatePolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificatePolicy> CertificateClient::UpdateCertificatePolicy(
    std::string const& name,
    CertificatePolicy const& certificatePolicy,
    Azure::Core::Context const& context) const
{
  auto payload = CertificatePolicySerializer::Serialize(certificatePolicy);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
  auto request
      = CreateRequest(HttpMethod::Patch, {CertificatesPath, name, PolicyPath}, &payloadStream);
  auto rawResponse = SendRequest(request, context);

  auto value = CertificatePolicySerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificatePolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<BackupCertificateResult> CertificateClient::BackupCertificate(
    std::string name,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Post, {CertificatesPath, name, BackupPath});
  auto rawResponse = SendRequest(request, context);

  auto value = BackupCertificateSerializer::Deserialize(*rawResponse);
  return Azure::Response<BackupCertificateResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::RestoreCertificateBackup(
    BackupCertificateResult const& backup,
    Azure::Core::Context const& context) const
{
  auto payload = BackupCertificateSerializer::Serialize(backup.Certificate);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(HttpMethod::Post, {CertificatesPath, RestorePath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize("", *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}
CertificatePropertiesPagedResponse CertificateClient::GetPropertiesOfCertificates(
    GetPropertiesOfCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  (void)options;
  // Request and settings
  auto request = ContinuationTokenRequest({CertificatesPath}, options.NextPageToken);
  if (options.IncludePending)
  {
    request.GetUrl().AppendQueryParameter(
        IncludePendingQuery, options.IncludePending.Value() ? TrueQueryValue : FalseQueryValue);
  }
  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = CertificatePropertiesPagedResponseSerializer::Deserialize(*rawResponse);
  return CertificatePropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<CertificateClient>(*this));
}

CertificatePropertiesPagedResponse CertificateClient::GetPropertiesOfCertificateVersions(
    std::string const& name,
    GetPropertiesOfCertificateVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request
      = ContinuationTokenRequest({CertificatesPath, name, VersionsPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = CertificatePropertiesPagedResponseSerializer::Deserialize(*rawResponse);
  return CertificatePropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<CertificateClient>(*this));
}

IssuerPropertiesPagedResponse CertificateClient::GetPropertiesOfIssuers(
    GetPropertiesOfIssuersOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({CertificatesPath, IssuersPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = IssuerPropertiesPagedResponseSerializer::Deserialize(*rawResponse);
  return IssuerPropertiesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<CertificateClient>(*this));
}

DeletedCertificatesPagedResponse CertificateClient::GetDeletedCertificates(
    GetDeletedCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({DeletedCertificatesPath}, options.NextPageToken);

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = DeletedCertificatesPagedResponseSerializer::Deserialize(*rawResponse);
  return DeletedCertificatesPagedResponse(
      std::move(value), std::move(rawResponse), std::make_unique<CertificateClient>(*this));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::ImportCertificate(
    std::string const& name,
    ImportCertificateOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = ImportCertificateOptionsSerializer::Serialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request
      = CreateRequest(HttpMethod::Post, {CertificatesPath, name, ImportPath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::MergeCertificate(
    std::string const& name,
    MergeCertificateOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = MergeCertificateOptionsSerializer::Serialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Post, {CertificatesPath, name, PendingPath, MergePath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize(name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::UpdateCertificateProperties(
    std::string const& name,
    std::string const& version,
    CertificateUpdateOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateUpdateOptionsSerializer::Serialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request
      = CreateRequest(HttpMethod::Patch, {CertificatesPath, name, version}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize(options.Properties.Name, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

const ServiceVersion ServiceVersion::V7_2("7.2");
