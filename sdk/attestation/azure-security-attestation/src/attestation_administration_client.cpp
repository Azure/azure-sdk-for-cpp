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
  auto resultToken
      = AttestationTokenInternal<Models::_detail::PolicyResult, PolicyResultSerializer>(
          responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  std::vector<AttestationSigner> const& signers = GetAttestationSigners(context);
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      signers);

  // Extract the underlying policy token from the response.
  std::string policyTokenValue
      = *static_cast<AttestationToken<Models::_detail::PolicyResult>>(resultToken)
            .Body.PolicyToken;

  // TPM policies are empty by default, at least in our test instances, so handle the empty policy
  // token case.
  auto policyTokenI
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
  auto returnedToken = AttestationTokenInternal<std::string>(responseToken, returnPolicy);
  return Response<AttestationToken<std::string>>(returnedToken, std::move(response));
}

Models::AttestationToken<std::nullptr_t>
AttestationAdministrationClient::CreateSetAttestationPolicyToken(
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

  auto tokenToSet(
      AttestationTokenInternal<StoredAttestationPolicy, StoredAttestationPolicySerializer>::
          CreateToken(storedPolicy, signingKey));
  auto tokenToSend(static_cast<AttestationToken<StoredAttestationPolicy>>(tokenToSet));

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
  // Calculate a signed (or unsigned) attestation policy token to send to the service.
  Models::AttestationToken<std::nullptr_t> tokenToSend(
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
  std::vector<AttestationSigner> const& signers = GetAttestationSigners(context);
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      signers);

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
    returnedResult.PolicyTokenHash
        = Base64Url::Base64UrlDecode(*internalResult.PolicyTokenHash);
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
  // Calculate a signed (or unsigned) attestation policy token to send to the service.
  Models::AttestationToken<std::nullptr_t> tokenToSend(
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
  std::vector<AttestationSigner> const& signers = GetAttestationSigners(context);
  resultToken.ValidateToken(
      options.TokenValidationOptions ? *options.TokenValidationOptions
                                     : this->m_tokenValidationOptions,
      signers);

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
    returnedResult.PolicyTokenHash
        = Base64Url::Base64UrlDecode(*internalResult.PolicyTokenHash);
  }

  // Construct a token whose body is the policy result, but whose token is the response from the
  // service.
  auto returnedToken
      = AttestationTokenInternal<Models::PolicyResult>(responseToken, returnedResult);
  return Response<AttestationToken<Models::PolicyResult>>(returnedToken, std::move(response));
}

/**
 * @brief Retrieve the attestation signers to validate the attestation token returned from the
 * service.
 *
 * @details Validating attestation tokens returned by the attestation service requires a set of
 * possible signers for the attestation token. Retrieving this can take a significant amount of time
 * (tens or hundreds of milliseconds), so we cache the results for the lifetime of this client.
 *
 * @param context Client context for the request to the service.
 * @return std::vector<AttestationSigner> const& Returns a reference to the private member filled in
 * with the signers returned by the service.
 */
std::vector<AttestationSigner> const& AttestationAdministrationClient::GetAttestationSigners(
    Azure::Core::Context const& context) const
{
  std::unique_lock<std::shared_timed_mutex> stateLock(SharedStateLock);

  if (m_attestationSigners.size() == 0)
  {
    auto request
        = AttestationCommonRequest::CreateRequest(m_endpoint, HttpMethod::Get, {"certs"}, nullptr);
    auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
    auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
    AttestationSigningCertificateResult returnValue;
    for (const auto& jwk : jsonWebKeySet.Keys)
    {
      AttestationSignerInternal internalSigner(jwk);
      m_attestationSigners.push_back(internalSigner);
    }
  }
  return m_attestationSigners;
}
