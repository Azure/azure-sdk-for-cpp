// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_administration_client.hpp"
#include "azure/attestation/attestation_client.hpp"
#include "private/attestation_client_models_private.hpp"
#include "private/attestation_client_private.hpp"
#include "private/attestation_common_request.hpp"
#include "private/attestation_deserializers_private.hpp"
#include "private/package_version.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <shared_mutex>

#include <string>

// cspell: words jwks
using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Security::Attestation::_detail;
using namespace Azure::Security::Attestation::Models::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;
using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::_internal;

const Models::AttestationType AttestationType::SgxEnclave("SgxEnclave");
const Models::AttestationType AttestationType::OpenEnclave("OpenEnclave");
const Models::AttestationType AttestationType::Tpm("Tpm");
const Models::PolicyModification PolicyModification::Removed("Removed");
const Models::PolicyModification PolicyModification::Updated("Updated");
const Models::PolicyCertificateModification PolicyCertificateModification::IsAbsent("IsAbsent");
const Models::PolicyCertificateModification PolicyCertificateModification::IsPresent("IsPresent");

AttestationAdministrationClient::AttestationAdministrationClient(
    std::string const& endpoint,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    AttestationAdministrationClientOptions const& options)
    : m_endpoint(endpoint), m_apiVersion(options.Version.ToString()),
      m_tokenValidationOptions(options.TokenValidationOptions)
{
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  if (credential)
  {
    m_credentials = credential;
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://attest.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }
  m_apiVersion = options.Version.ToString();
  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  // This configuration will make winHTTP to disable client certificate for all attestation requests
  perCallpolicies.emplace_back(std::make_unique<SetNoClientCertificatePolicy>());
#endif

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      "Attestation",
      PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}

namespace {
std::shared_timed_mutex SharedStateLock;
}

Azure::Response<Models::AttestationToken<std::string>>
AttestationAdministrationClient::GetAttestationPolicy(
    AttestationType const& attestationType,
    GetPolicyOptions const& options,
    Azure::Core::Context const& context) const
{
  CheckAttestationSigners();

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint,
      m_apiVersion,
      HttpMethod::Get,
      {"policies/" + attestationType.ToString()},
      nullptr);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);

  // Deserialize the Service response token and return the JSON web token returned by the
  // service.
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);

  // Parse the JWT returned by the attestation service.
  const auto resultToken
      = AttestationTokenInternal<Models::_detail::PolicyResult, PolicyResultSerializer>(
          responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      m_attestationSigners);

  // Extract the underlying policy token from the response.
  std::string policyTokenValue
      = *static_cast<AttestationToken<Models::_detail::PolicyResult>>(resultToken).Body.PolicyToken;

  // TPM policies are empty by default, at least in our test instances, so handle the empty policy
  // token case.
  const auto policyTokenI
      = AttestationTokenInternal<StoredAttestationPolicy, StoredAttestationPolicySerializer>(
          policyTokenValue);
  AttestationToken<StoredAttestationPolicy> policyToken(policyTokenI);
  std::string returnPolicy;
  if (policyToken.Body.AttestationPolicy)
  {
    std::vector<uint8_t> policyUtf8 = *policyToken.Body.AttestationPolicy;
    returnPolicy = std::string(policyUtf8.begin(), policyUtf8.end());
  }

  // Construct a token whose body is the policy, but whose token is the response from the
  // service.
  const auto returnedToken = AttestationTokenInternal<std::string>(responseToken, returnPolicy);
  return Response<AttestationToken<std::string>>(returnedToken, std::move(response));
}

Models::AttestationToken<> AttestationAdministrationClient::CreateSetAttestationPolicyToken(
    Azure::Nullable<std::string> const& newAttestationPolicy,
    Azure::Nullable<AttestationSigningKey> const& signingKey) const
{
  // Embed the encoded policy in the StoredAttestationPolicy.
  Azure::Nullable<StoredAttestationPolicy> storedPolicy;
  if (newAttestationPolicy)
  {
    storedPolicy = StoredAttestationPolicy{
        std::vector<uint8_t>(newAttestationPolicy->begin(), newAttestationPolicy->end())};
  }

  const auto tokenToSet(
      AttestationTokenInternal<StoredAttestationPolicy, StoredAttestationPolicySerializer>::
          CreateToken(storedPolicy, signingKey));
  const auto tokenToSend(static_cast<AttestationToken<StoredAttestationPolicy>>(tokenToSet));

  // Strip the body type off the returned JWS - the caller of the function doesn't need it.
  return AttestationTokenInternal<std::nullptr_t>(tokenToSend.RawToken);
}

Azure::Response<Models::AttestationToken<Models::PolicyResult>>
AttestationAdministrationClient::SetAttestationPolicy(
    AttestationType const& attestationType,
    std::string const& newAttestationPolicy,
    SetPolicyOptions const& options,
    Azure::Core::Context const& context) const
{
  CheckAttestationSigners();
  // Calculate a signed (or unsigned) attestation policy token to send to the service.
  Models::AttestationToken<> const tokenToSend(
      CreateSetAttestationPolicyToken(newAttestationPolicy, options.SigningKey));

  Azure::Core::IO::MemoryBodyStream stream(
      reinterpret_cast<uint8_t const*>(tokenToSend.RawToken.data()), tokenToSend.RawToken.size());

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint,
      m_apiVersion,
      HttpMethod::Put,
      {"policies/" + attestationType.ToString()},
      &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);

  // Deserialize the Service response token and return the JSON web token returned by the
  // service.
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);

  // Parse the JWT returned by the attestation service.
  auto resultToken
      = AttestationTokenInternal<Models::_detail::PolicyResult, PolicyResultSerializer>(
          responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      m_attestationSigners);

  // Extract the underlying policy token from the response.
  auto internalResult
      = static_cast<AttestationToken<Models::_detail::PolicyResult>>(resultToken).Body;

  Models::PolicyResult returnedResult;
  if (internalResult.PolicyResolution)
  {
    returnedResult.PolicyResolution = Models::PolicyModification(*internalResult.PolicyResolution);
  }
  if (internalResult.PolicySigner)
  {
    returnedResult.PolicySigner = AttestationSignerInternal(*internalResult.PolicySigner);
  }
  if (internalResult.PolicyTokenHash)
  {
    returnedResult.PolicyTokenHash = Base64Url::Base64UrlDecode(*internalResult.PolicyTokenHash);
  }

  // Construct a token whose body is the policy result, but whose token is the response from the
  // service.
  auto returnedToken
      = AttestationTokenInternal<Models::PolicyResult>(responseToken, returnedResult);
  return Response<AttestationToken<Models::PolicyResult>>(returnedToken, std::move(response));
}

Azure::Response<Models::AttestationToken<Models::PolicyResult>>
AttestationAdministrationClient::ResetAttestationPolicy(
    AttestationType const& attestationType,
    SetPolicyOptions const& options,
    Azure::Core::Context const& context) const
{
  CheckAttestationSigners();
  // Calculate a signed (or unsigned) attestation policy token to send to the service.
  Models::AttestationToken<> tokenToSend(
      CreateSetAttestationPolicyToken(Azure::Nullable<std::string>(), options.SigningKey));

  Azure::Core::IO::MemoryBodyStream stream(
      reinterpret_cast<uint8_t const*>(tokenToSend.RawToken.data()), tokenToSend.RawToken.size());

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint,
      m_apiVersion,
      HttpMethod::Post,
      {"policies/" + attestationType.ToString() + ":reset"},
      &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);

  // Deserialize the Service response token and return the JSON web token returned by the
  // service.
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);

  // Parse the JWT returned by the attestation service.
  auto resultToken
      = AttestationTokenInternal<Models::_detail::PolicyResult, PolicyResultSerializer>(
          responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      m_attestationSigners);

  // Extract the underlying policy token from the response.
  auto internalResult
      = static_cast<AttestationToken<Models::_detail::PolicyResult>>(resultToken).Body;

  Models::PolicyResult returnedResult;
  if (internalResult.PolicyResolution)
  {
    returnedResult.PolicyResolution = Models::PolicyModification(*internalResult.PolicyResolution);
  }
  // Note that the attestation service currently never returns these values on Reset, even though
  // they are meaningful. Commenting them out to improve code coverage numbers. At some point the
  // attestation service may start returning these values, at which point they can be un-commented
  // out.
  //  if (internalResult.PolicySigner)
  //  {
  //    returnedResult.PolicySigner = AttestationSignerInternal(*internalResult.PolicySigner);
  //  }
  //  if (internalResult.PolicyTokenHash)
  //  {
  //    returnedResult.PolicyTokenHash =
  //    Base64Url::Base64UrlDecode(*internalResult.PolicyTokenHash);
  //  }

  // Construct a token whose body is the policy result, but whose token is the response from the
  // service.
  auto returnedToken
      = AttestationTokenInternal<Models::PolicyResult>(responseToken, returnedResult);
  return Response<AttestationToken<Models::PolicyResult>>(returnedToken, std::move(response));
}

Azure::Response<Models::AttestationToken<PolicyCertificateListResult>>
AttestationAdministrationClient::GetPolicyManagementCertificates(
    GetPolicyManagementCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  CheckAttestationSigners();

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Get, {"certificates"}, nullptr);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);

  // Deserialize the Service response token and return the JSON web token returned by the
  // service.
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);

  // Parse the JWT returned by the attestation service.
  auto resultToken = AttestationTokenInternal<
      Models::_detail::GetPolicyCertificatesResult,
      PolicyCertificateGetResultSerializer>(responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      m_attestationSigners);

  Models::_detail::JsonWebKeySet jwks(
      *static_cast<AttestationToken<Models::_detail::GetPolicyCertificatesResult>>(resultToken)
           .Body.PolicyCertificates);
  Models::PolicyCertificateListResult returnedResult;
  for (const auto& certificate : jwks.Keys)
  {
    returnedResult.Certificates.push_back(AttestationSignerInternal(certificate));
  }

  // Construct a token whose body is the get policy certificates result, but whose token is the
  // response from the service.
  auto returnedToken = AttestationTokenInternal<Models::PolicyCertificateListResult>(
      responseToken, returnedResult);
  return Response<AttestationToken<Models::PolicyCertificateListResult>>(
      returnedToken, std::move(response));
}

std::string AttestationAdministrationClient::CreatePolicyCertificateModificationToken(
    std::string const& pemEncodedX509CertificateToAdd,
    AttestationSigningKey const& existingSigningKey) const
{
  CheckAttestationSigners();

  // Calculate a signed attestation policy token to send to the service.
  // Embed the encoded policy in the StoredAttestationPolicy.
  const auto x5cToAdd(Cryptography::ImportX509Certificate(pemEncodedX509CertificateToAdd));

  // Create a JWK to add to the body.
  JsonWebKey jwkToSend;
  jwkToSend.Kty = x5cToAdd->GetKeyType();
  jwkToSend.X5c = std::vector<std::string>{x5cToAdd->ExportAsBase64()};

  PolicyCertificateManagementBody bodyToSend{jwkToSend};

  auto const internalTokenToSend(
      AttestationTokenInternal<
          PolicyCertificateManagementBody,
          PolicyCertificateManagementBodySerializer>::CreateToken(bodyToSend, existingSigningKey));

  auto const tokenToSend(
      static_cast<AttestationToken<PolicyCertificateManagementBody>>(internalTokenToSend));

  // JSON encode the string we're going to send.
  return Azure::Core::Json::_internal::json(tokenToSend.RawToken).dump();
}

Models::AttestationToken<Models::PolicyCertificateModificationResult>
AttestationAdministrationClient::ProcessPolicyCertModificationResult(
    std::unique_ptr<RawResponse> const& serverResponse,
    AttestationTokenValidationOptions const& tokenValidationOptions) const
{
  CheckAttestationSigners();

  // Deserialize the Service response token and return the JSON web token returned by the
  // service.
  std::string responseToken
      = AttestationServiceTokenResponseSerializer::Deserialize(serverResponse);

  // Parse the JWT returned by the attestation service.
  auto const resultToken = AttestationTokenInternal<
      Models::_detail::ModifyPolicyCertificatesResult,
      ModifyPolicyCertificatesResultSerializer>(responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  resultToken.ValidateToken(tokenValidationOptions, m_attestationSigners);

  // Extract the underlying policy token from the response.
  auto internalResult
      = static_cast<AttestationToken<Models::_detail::ModifyPolicyCertificatesResult>>(resultToken)
            .Body;

  Models::PolicyCertificateModificationResult returnValue;
  if (internalResult.CertificateResolution)
  {
    returnValue.CertificateModification
        = Models::PolicyCertificateModification(*internalResult.CertificateResolution);
  }
  if (internalResult.CertificateThumbprint)
  {
    returnValue.CertificateThumbprint = (*internalResult.CertificateThumbprint);
  }

  // Construct a token whose body is the policy result, but whose token is the response from the
  // service.
  auto const returnedToken = AttestationTokenInternal<Models::PolicyCertificateModificationResult>(
      responseToken, returnValue);
  return returnedToken;
}

Azure::Response<Models::AttestationToken<Models::PolicyCertificateModificationResult>>
AttestationAdministrationClient::AddPolicyManagementCertificate(
    std::string const& pemEncodedX509CertificateToAdd,
    AttestationSigningKey const& existingSigningKey,
    AddPolicyManagementCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  CheckAttestationSigners();

  auto const policyCertToken(
      CreatePolicyCertificateModificationToken(pemEncodedX509CertificateToAdd, existingSigningKey));
  Azure::Core::IO::MemoryBodyStream stream(
      reinterpret_cast<uint8_t const*>(policyCertToken.data()), policyCertToken.size());

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"certificates:add"}, &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  AttestationToken<PolicyCertificateModificationResult> returnValue(
      ProcessPolicyCertModificationResult(
          response,
          options.TokenValidationOptions ? *options.TokenValidationOptions
                                         : this->m_tokenValidationOptions));
  return Response<AttestationToken<Models::PolicyCertificateModificationResult>>(
      returnValue, std::move(response));
}

Azure::Response<Models::AttestationToken<Models::PolicyCertificateModificationResult>>
AttestationAdministrationClient::RemovePolicyManagementCertificate(
    std::string const& pemEncodedX509CertificateToRemove,
    AttestationSigningKey const& existingSigningKey,
    AddPolicyManagementCertificatesOptions const& options,
    Azure::Core::Context const& context) const
{
  CheckAttestationSigners();

  // Calculate a signed (or unsigned) attestation policy token to send to the service.
  // Embed the encoded policy in the StoredAttestationPolicy.
  auto const policyCertToken(CreatePolicyCertificateModificationToken(
      pemEncodedX509CertificateToRemove, existingSigningKey));

  Azure::Core::IO::MemoryBodyStream stream(
      reinterpret_cast<uint8_t const*>(policyCertToken.data()), policyCertToken.size());

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"certificates:remove"}, &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  AttestationToken<PolicyCertificateModificationResult> returnValue(
      ProcessPolicyCertModificationResult(
          response,
          options.TokenValidationOptions ? *options.TokenValidationOptions
                                         : this->m_tokenValidationOptions));
  return Response<AttestationToken<Models::PolicyCertificateModificationResult>>(
      returnValue, std::move(response));
}

/**
 * @brief Retrieves the information needed to validate the response returned from the attestation
 * service.
 *
 * @details Validating the response returned by the attestation service requires a set of
 * possible signers for the attestation token.
 *
 * @param context Client context for the request to the service.
 */
void AttestationAdministrationClient::RetrieveResponseValidationCollateral(
    Azure::Core::Context const& context) const
{
  std::unique_lock<std::shared_timed_mutex> stateLock(SharedStateLock);

  if (m_attestationSigners.empty())
  {
    stateLock.unlock();
    auto request
        = AttestationCommonRequest::CreateRequest(m_endpoint, HttpMethod::Get, {"certs"}, nullptr);
    auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
    auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
    AttestationSigningCertificateResult returnValue;
    std::vector<AttestationSigner> newValue;
    for (const auto& jwk : jsonWebKeySet.Keys)
    {
      AttestationSignerInternal internalSigner(jwk);
      newValue.push_back(internalSigner);
    }
    stateLock.lock();
    if (m_attestationSigners.empty())
    {
      m_attestationSigners = newValue;
    }
  }
}

void AttestationAdministrationClient::CheckAttestationSigners() const
{
  std::unique_lock<std::shared_timed_mutex> stateLock(SharedStateLock);

  AZURE_ASSERT_MSG(
      !m_attestationSigners.empty(),
      "RetrieveResponseValidationCollateral must be called before this API.");
}
