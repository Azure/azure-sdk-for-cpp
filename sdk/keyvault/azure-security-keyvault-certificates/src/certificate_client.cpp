//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/certificates/certificate_client.hpp"

#include "azure/keyvault/shared/keyvault_shared.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"
#include "private/keyvault_certificates_common_request.hpp"
#include "private/package_version.hpp"
#include <azure/core/base64.hpp>

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
    : m_vaultUrl(vaultUrl), m_apiVersion(options.ApiVersion)
{
  auto apiVersion = options.ApiVersion;

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
    std::string const& certificateName,
    Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, certificateName});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultCertificateSerializer::Deserialize(certificateName, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Response<KeyVaultCertificate> CertificateClient::GetCertificateVersion(
    std::string const& certificateName,
    std::string const& certificateVersion,
    Context const& context) const
{
  // Request with no payload
  std::vector<std::string> path{{CertificatesPath, certificateName, certificateVersion}};

  auto request = CreateRequest(HttpMethod::Get, std::move(path));

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = _detail::KeyVaultCertificateSerializer::Deserialize(certificateName, *rawResponse);
  return Azure::Response<KeyVaultCertificate>(std::move(value), std::move(rawResponse));
}

CreateCertificateOperation CertificateClient::StartCreateCertificate(
    std::string const& certificateName,
    CertificateCreateOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateCreateOptionsSerializer::Serialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Post,
      {CertificatesPath, certificateName, CertificatesCreatePath},
      &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = _detail::CertificateOperationSerializer::Deserialize(*rawResponse);

  return CreateCertificateOperation(value.Name, std::make_shared<CertificateClient>(*this));
}

Response<DeletedCertificate> CertificateClient::GetDeletedCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {DeletedCertificatesPath, certificateName});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  auto value = DeletedCertificateSerializer::Deserialize(certificateName, *rawResponse);
  return Azure::Response<DeletedCertificate>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::GetIssuer(
    std::string const& issuerName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, IssuersPath, issuerName});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateIssuerSerializer::Deserialize(issuerName, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::DeleteIssuer(
    std::string const& issuerName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, IssuersPath, issuerName});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateIssuerSerializer::Deserialize(issuerName, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::CreateIssuer(
    std::string const& issuerName,
    CertificateIssuer const& certificateIssuer,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateIssuerSerializer::Serialize(certificateIssuer);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request
      = CreateRequest(HttpMethod::Put, {CertificatesPath, IssuersPath, issuerName}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = CertificateIssuerSerializer::Deserialize(issuerName, *rawResponse);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::UpdateIssuer(
    std::string const& issuerName,
    CertificateIssuer const& certificateIssuer,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateIssuerSerializer::Serialize(certificateIssuer);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Patch, {CertificatesPath, IssuersPath, issuerName}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = CertificateIssuerSerializer::Deserialize(issuerName, *rawResponse);
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
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, certificateName, PendingPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateOperationProperties>
CertificateClient::CancelPendingCertificateOperation(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  CertificateOperationUpdateOptions option;
  option.CancelationRequested = true;
  auto payload = CertificateOperationUpdateOptionSerializer::Serialize(option);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Patch, {CertificatesPath, certificateName, PendingPath}, &payloadStream);
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificateOperationProperties>
CertificateClient::DeletePendingCertificateOperation(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request
      = CreateRequest(HttpMethod::Delete, {CertificatesPath, certificateName, PendingPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificateOperationSerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(rawResponse));
}

Response<PurgedCertificate> CertificateClient::PurgeDeletedCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {DeletedCertificatesPath, certificateName});

  // Send and parse respone
  auto rawResponse = SendRequest(request, context);
  PurgedCertificate value;
  return Azure::Response<PurgedCertificate>(std::move(value), std::move(rawResponse));
}

DeleteCertificateOperation CertificateClient::StartDeleteCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Delete, {CertificatesPath, certificateName});

  auto rawResponse = SendRequest(request, context);
  auto value = DeletedCertificate();
  value.Properties.Name = certificateName;
  auto responseT = Azure::Response<DeletedCertificate>(std::move(value), std::move(rawResponse));
  return DeleteCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}

RecoverDeletedCertificateOperation CertificateClient::StartRecoverDeletedCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request
      = CreateRequest(HttpMethod::Post, {DeletedCertificatesPath, certificateName, RecoverPath});

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateWithPolicy();
  value.Properties.Name = certificateName;
  auto responseT
      = Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
  return RecoverDeletedCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}
Azure::Response<CertificatePolicy> CertificateClient::GetCertificatePolicy(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {CertificatesPath, certificateName, PolicyPath});
  auto rawResponse = SendRequest(request, context);

  auto value = CertificatePolicySerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificatePolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<CertificatePolicy> CertificateClient::UpdateCertificatePolicy(
    std::string const& certificateName,
    CertificatePolicy const& certificatePolicy,
    Azure::Core::Context const& context) const
{
  auto payload = CertificatePolicySerializer::Serialize(certificatePolicy);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
  auto request = CreateRequest(
      HttpMethod::Patch, {CertificatesPath, certificateName, PolicyPath}, &payloadStream);
  auto rawResponse = SendRequest(request, context);

  auto value = CertificatePolicySerializer::Deserialize(*rawResponse);
  return Azure::Response<CertificatePolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<BackupCertificateResult> CertificateClient::BackupCertificate(
    std::string certificateName,
    Azure::Core::Context const& context) const
{
  auto request = CreateRequest(HttpMethod::Post, {CertificatesPath, certificateName, BackupPath});
  auto rawResponse = SendRequest(request, context);

  auto value = BackupCertificateSerializer::Deserialize(*rawResponse);
  return Azure::Response<BackupCertificateResult>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::RestoreCertificateBackup(
    std::vector<uint8_t> const& certificateBackup,
    Azure::Core::Context const& context) const
{
  auto payload = BackupCertificateSerializer::Serialize(certificateBackup);
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
    std::string const& certificateName,
    GetPropertiesOfCertificateVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest(
      {CertificatesPath, certificateName, VersionsPath}, options.NextPageToken);

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
    std::string const& certificateName,
    ImportCertificateOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = ImportCertificateOptionsSerializer::Serialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Post, {CertificatesPath, certificateName, ImportPath}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize(certificateName, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::MergeCertificate(
    std::string const& certificateName,
    MergeCertificateOptions const& options,
    Azure::Core::Context const& context) const
{
  auto payload = MergeCertificateOptionsSerializer::Serialize(options);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Post,
      {CertificatesPath, certificateName, PendingPath, MergePath},
      &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize(certificateName, *rawResponse);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(rawResponse));
}

Azure::Response<KeyVaultCertificate> CertificateClient::UpdateCertificateProperties(
    std::string const& certificateName,
    std::string const& certificateVersion,
    CertificateProperties const& certificateProperties,
    Azure::Core::Context const& context) const
{
  auto payload = CertificateUpdateOptionsSerializer::Serialize(certificateProperties);
  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(payload.data()), payload.size());

  auto request = CreateRequest(
      HttpMethod::Patch, {CertificatesPath, certificateName, certificateVersion}, &payloadStream);

  auto rawResponse = SendRequest(request, context);
  auto value = KeyVaultCertificateSerializer::Deserialize(certificateName, *rawResponse);
  return Azure::Response<KeyVaultCertificate>(std::move(value), std::move(rawResponse));
}
