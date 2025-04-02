// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/certificates/certificate_client.hpp"

#include "azure/keyvault/shared/keyvault_challenge_based_auth.hpp"
#include "azure/keyvault/shared/keyvault_shared.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"
#include "private/keyvault_certificates_common_request.hpp"
#include "private/package_version.hpp"
#include "generated/key_vault_client.hpp"
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
    std::shared_ptr<const Core::Credentials::TokenCredential> credential,
    CertificateClientOptions options)
    : m_vaultUrl(vaultUrl), m_apiVersion(options.ApiVersion)
{
  _detail::KeyVaultClientOptions generatedClientOptions;
  static_cast<Core::_internal::ClientOptions&>(generatedClientOptions)
      = static_cast<const Core::_internal::ClientOptions&>(options);
  generatedClientOptions.ApiVersion = options.ApiVersion;
  m_client = std::make_shared<_detail::KeyVaultClient>(
      _detail::KeyVaultClient(vaultUrl, credential, generatedClientOptions));
}

Response<KeyVaultCertificateWithPolicy> CertificateClient::GetCertificate(
    std::string const& certificateName,
    Context const& context) const
{
  auto result = m_client->GetCertificate(certificateName,"", context);
  auto value = KeyVaultCertificateWithPolicy(result.Value);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(result.RawResponse));
}

Response<KeyVaultCertificate> CertificateClient::GetCertificateVersion(
    std::string const& certificateName,
    std::string const& certificateVersion,
    Context const& context) const
{
  auto result = m_client->GetCertificate(certificateName, certificateVersion, context);

  auto value = KeyVaultCertificate(result.Value);
  return Azure::Response<KeyVaultCertificate>(std::move(value), std::move(result.RawResponse));
}

CreateCertificateOperation CertificateClient::StartCreateCertificate(
    std::string const& certificateName,
    CertificateCreateOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::Models::CertificateCreateParameters parameters
      = (const_cast < CertificateCreateOptions&>( options)).ToCertificateCreateParameters();
  auto result = m_client->CreateCertificate(certificateName, parameters, context);
  return CreateCertificateOperation(certificateName, std::make_shared<CertificateClient>(*this));
}

Response<DeletedCertificate> CertificateClient::GetDeletedCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetDeletedCertificate(certificateName, context);
  auto value = DeletedCertificate(result.Value);
  return Azure::Response<DeletedCertificate>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::GetIssuer(
    std::string const& issuerName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetCertificateIssuer(issuerName, context);

  auto value = CertificateIssuer(issuerName, result.Value);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::DeleteIssuer(
    std::string const& issuerName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->DeleteCertificateIssuer(issuerName, context);

  auto value = CertificateIssuer(issuerName, result.Value);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::CreateIssuer(
    std::string const& issuerName,
    CertificateIssuer const& certificateIssuer,
    Azure::Core::Context const& context) const
{
  _detail::Models::CertificateIssuerSetParameters issuerParameters
      = (const_cast<CertificateIssuer&>(certificateIssuer)).ToCertificateIssuerSetParameters();
  auto result = m_client->SetCertificateIssuer(issuerName, issuerParameters, context);

  auto value = CertificateIssuer(issuerName, result.Value);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateIssuer> CertificateClient::UpdateIssuer(
    std::string const& issuerName,
    CertificateIssuer const& certificateIssuer,
    Azure::Core::Context const& context) const
{
  _detail::Models::CertificateIssuerUpdateParameters issuerParameters
      = (const_cast<CertificateIssuer&>(certificateIssuer)).ToCertificateIssuerUpdateParameters();
  auto result = m_client->UpdateCertificateIssuer(
      issuerName, issuerParameters, context);

  auto value = CertificateIssuer(issuerName, result.Value);
  return Azure::Response<CertificateIssuer>(std::move(value), std::move(result.RawResponse));
}

Response<CertificateContactsResult> CertificateClient::GetContacts(
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetCertificateContacts(context);

  auto value = CertificateContactsResult(result.Value);
  return Azure::Response<CertificateContactsResult>(std::move(value), std::move(result.RawResponse));
}

Response<CertificateContactsResult> CertificateClient::DeleteContacts(
    Azure::Core::Context const& context) const
{
  auto result = m_client->DeleteCertificateContacts(context);

  auto value = CertificateContactsResult(result.Value);
  return Azure::Response<CertificateContactsResult>(std::move(value), std::move(result.RawResponse));
}

Response<CertificateContactsResult> CertificateClient::SetContacts(
    std::vector<CertificateContact> const& contacts,
    Azure::Core::Context const& context) const
{
  _detail::Models::Contacts setContacts;
  setContacts.ContactList = std::vector<_detail::Models::Contact>();
  for (auto& contact : contacts)
  {
    _detail::Models::Contact setContact;
    setContact.EmailAddress = contact.EmailAddress;
    setContact.Name = contact.Name;
    setContact.Phone = contact.Phone;
    setContacts.ContactList.Value().emplace_back(setContact);
  }
  auto result = m_client->SetCertificateContacts(setContacts, context);

  auto value = CertificateContactsResult(result.Value);
  return Azure::Response<CertificateContactsResult>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateOperationProperties> CertificateClient::GetPendingCertificateOperation(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetCertificateOperation(certificateName, context);    
  auto value = CertificateOperationProperties(result.Value);
  value.Name = certificateName;
  value.VaultUrl = m_vaultUrl.GetAbsoluteUrl();
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateOperationProperties>
CertificateClient::CancelPendingCertificateOperation(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetCertificateOperation(certificateName, context);
  auto value = CertificateOperationProperties(result.Value);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificateOperationProperties>
CertificateClient::DeletePendingCertificateOperation(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->DeleteCertificateOperation(certificateName, context);
  auto value = CertificateOperationProperties(result.Value);
  return Azure::Response<CertificateOperationProperties>(std::move(value), std::move(result.RawResponse));
}

Response<PurgedCertificate> CertificateClient::PurgeDeletedCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->PurgeDeletedCertificate(certificateName, context);
  PurgedCertificate value;
  return Azure::Response<PurgedCertificate>(std::move(value), std::move(result.RawResponse));
}

DeleteCertificateOperation CertificateClient::StartDeleteCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->DeleteCertificate(certificateName, context);
  auto value = DeletedCertificate(result.Value);
  value.Properties.Name = certificateName;
  auto responseT = Azure::Response<DeletedCertificate>(std::move(value), std::move(result.RawResponse));
  return DeleteCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}

RecoverDeletedCertificateOperation CertificateClient::StartRecoverDeletedCertificate(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->RecoverDeletedCertificate(certificateName, context);
  auto value = KeyVaultCertificateWithPolicy(result.Value);
  value.Properties.Name = certificateName;
  auto responseT
      = Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(result.RawResponse));
  return RecoverDeletedCertificateOperation(
      std::make_shared<CertificateClient>(*this), std::move(responseT));
}
Azure::Response<CertificatePolicy> CertificateClient::GetCertificatePolicy(
    std::string const& certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->GetCertificatePolicy(certificateName, context);
  auto value = CertificatePolicy(result.Value);
  return Azure::Response<CertificatePolicy>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<CertificatePolicy> CertificateClient::UpdateCertificatePolicy(
    std::string const& certificateName,
    CertificatePolicy const& certificatePolicy,
    Azure::Core::Context const& context) const
{
  auto updatePolicy 
      = (const_cast<CertificatePolicy&>(certificatePolicy)).ToCertificatePolicy();
  auto result = m_client->UpdateCertificatePolicy(certificateName, updatePolicy, context);

  auto value = CertificatePolicy(result.Value);
  return Azure::Response<CertificatePolicy>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<BackupCertificateResult> CertificateClient::BackupCertificate(
    std::string certificateName,
    Azure::Core::Context const& context) const
{
  auto result = m_client->BackupCertificate(certificateName, context);
  BackupCertificateResult value;
  if (result.Value.Value.HasValue())
  {
    value.Certificate = result.Value.Value.Value();
  }
  return Azure::Response<BackupCertificateResult>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::RestoreCertificateBackup(
    std::vector<uint8_t> const& certificateBackup,
    Azure::Core::Context const& context) const
{
  _detail::Models::CertificateRestoreParameters restoreParameters;
  restoreParameters.CertificateBundleBackup = certificateBackup;
  auto result = m_client->RestoreCertificate(restoreParameters, context);
  auto value = KeyVaultCertificateWithPolicy(result.Value);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(result.RawResponse));
}

CertificatePropertiesPagedResponse CertificateClient::GetPropertiesOfCertificates(
    GetPropertiesOfCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  KeyVaultClientGetCertificatesOptions getOptions;
  getOptions.IncludePending = options.IncludePending;
  if (options.NextPageToken.HasValue())
  {
    getOptions.NextPageToken = options.NextPageToken.Value();
  }
  auto result = m_client->GetCertificates(getOptions, context);
  auto value = CertificatePropertiesPagedResponse(result);
  return CertificatePropertiesPagedResponse(
      std::move(value), std::move(result.RawResponse), std::make_unique<CertificateClient>(*this));
}

CertificatePropertiesPagedResponse CertificateClient::GetPropertiesOfCertificateVersions(
    std::string const& certificateName,
    GetPropertiesOfCertificateVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  KeyVaultClientGetCertificateVersionsOptions getOptions;
  if (options.NextPageToken.HasValue())
  {
    getOptions.NextPageToken = options.NextPageToken.Value();
  }
  auto result = m_client->GetCertificateVersions(certificateName, getOptions, context);
  auto value = CertificatePropertiesPagedResponse(result);
  return CertificatePropertiesPagedResponse(
      std::move(value), std::move(result.RawResponse), std::make_unique<CertificateClient>(*this));
}

IssuerPropertiesPagedResponse CertificateClient::GetPropertiesOfIssuers(
    GetPropertiesOfIssuersOptions const& options,
    Azure::Core::Context const& context) const
{
  // Request and settings
  auto request = ContinuationTokenRequest({CertificatesPath, IssuersPath}, options.NextPageToken);

  // Send and parse response
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

  // Send and parse response
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
  _detail::Models::CertificateImportParameters parameters
      = (const_cast<ImportCertificateOptions&>(options)).ToCertificateImportParameters();
  auto result = m_client->ImportCertificate(
      certificateName, parameters, context);
  auto value = KeyVaultCertificateWithPolicy(result.Value);
  return Azure::Response<KeyVaultCertificateWithPolicy>(std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyVaultCertificateWithPolicy> CertificateClient::MergeCertificate(
    std::string const& certificateName,
    MergeCertificateOptions const& options,
    Azure::Core::Context const& context) const
{
  _detail::Models::CertificateMergeParameters parameters
      = (const_cast<MergeCertificateOptions&>(options)).ToCertificateMergeParameters();

  auto result = m_client->MergeCertificate(certificateName, parameters, context);
  auto value = KeyVaultCertificateWithPolicy(result.Value);
  return Azure::Response<KeyVaultCertificateWithPolicy>(
      std::move(value), std::move(result.RawResponse));
}

Azure::Response<KeyVaultCertificate> CertificateClient::UpdateCertificateProperties(
    std::string const& certificateName,
    std::string const& certificateVersion,
    CertificateProperties const& certificateProperties,
    Azure::Core::Context const& context) const
{
  auto updeateProperties
      = (const_cast<CertificateProperties&>(certificateProperties)).ToCertificateUpdateParameters();
  auto result = m_client->UpdateCertificate(
      certificateName,
      certificateVersion,
      updeateProperties,
      context);
  auto value = KeyVaultCertificate(result.Value);
  return Azure::Response<KeyVaultCertificate>(std::move(value), std::move(result.RawResponse));
}
