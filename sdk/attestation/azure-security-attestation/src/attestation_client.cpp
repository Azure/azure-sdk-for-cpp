// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_client.hpp"
#include "azure/attestation/attestation_administration_client.hpp"
#include "private/attestation_client_models_private.hpp"
#include "private/attestation_client_private.hpp"
#include "private/attestation_common_request.hpp"
#include "private/attestation_deserializers_private.hpp"
#include "private/package_version.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/credentials/credentials.hpp>
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

AttestationClient::AttestationClient(
    std::string const& endpoint,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    AttestationClientOptions options)
    : m_endpoint(endpoint), m_credentials(credential),
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

Azure::Response<OpenIdMetadata> AttestationClient::GetOpenIdMetadata(
    Azure::Core::Context const& context) const
{
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, HttpMethod::Get, {".well-known/openid-configuration"}, nullptr);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  auto openIdMetadata(OpenIdMetadataSerializer::Deserialize(response));
  return Response<OpenIdMetadata>(std::move(openIdMetadata), std::move(response));
}

Azure::Response<TokenValidationCertificateResult> AttestationClient::GetTokenValidationCertificates(
    Azure::Core::Context const& context) const
{
  auto request
      = AttestationCommonRequest::CreateRequest(m_endpoint, HttpMethod::Get, {"certs"}, nullptr);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
  TokenValidationCertificateResult returnValue;
  for (const auto& jwk : jsonWebKeySet.Keys)
  {
    AttestationSignerInternal internalSigner(jwk);
    returnValue.Signers.push_back(internalSigner);
  }
  return Response<TokenValidationCertificateResult>(returnValue, std::move(response));
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestSgxEnclave(
    std::vector<uint8_t> const& sgxQuote,
    AttestSgxEnclaveOptions options,
    Azure::Core::Context const& context) const
{
  AttestSgxEnclaveRequest attestRequest{
      sgxQuote,
      options.InitTimeData,
      options.RunTimeData,
      options.DraftPolicyForAttestation,
      options.Nonce};

  const std::string serializedRequest(AttestSgxEnclaveRequestSerializer::Serialize(attestRequest));

  const auto encodedVector
      = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
  Azure::Core::IO::MemoryBodyStream stream(encodedVector);
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/SgxEnclave"}, &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);

  // Deserialize the Service response token and return the JSON web token returned by the service.
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);

  // Parse the JWT returned by the attestation service.
  auto const token
      = AttestationTokenInternal<AttestationResult, AttestationResultSerializer>(responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  token.ValidateToken(
      options.TokenValidationOptionsOverride ? *options.TokenValidationOptionsOverride
                                             : this->m_tokenValidationOptions,
      m_attestationSigners);

  // And return the attestation result to the caller.
  auto returnedToken = AttestationToken<AttestationResult>(token);
  return Response<AttestationToken<AttestationResult>>(returnedToken, std::move(response));
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestOpenEnclave(
    std::vector<uint8_t> const& openEnclaveReport,
    AttestOpenEnclaveOptions options,
    Azure::Core::Context const& context) const
{
  AttestOpenEnclaveRequest attestRequest{
      openEnclaveReport,
      options.InitTimeData,
      options.RunTimeData,
      options.DraftPolicyForAttestation,
      options.Nonce};
  std::string serializedRequest(AttestOpenEnclaveRequestSerializer::Serialize(attestRequest));

  auto encodedVector = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
  Azure::Core::IO::MemoryBodyStream stream(encodedVector);
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/OpenEnclave"}, &stream);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);
  auto token
      = AttestationTokenInternal<AttestationResult, AttestationResultSerializer>(responseToken);
  token.ValidateToken(
      options.TokenValidationOptionsOverride ? *options.TokenValidationOptionsOverride
                                             : this->m_tokenValidationOptions,
      m_attestationSigners);

  return Response<AttestationToken<AttestationResult>>(token, std::move(response));
}

Azure::Response<TpmAttestationResult> AttestationClient::AttestTpm(
    AttestTpmOptions const& attestTpmOptions,
    Azure::Core::Context const& context) const
{
  std::string jsonToSend = TpmDataSerializer::Serialize(attestTpmOptions.Payload);
  auto encodedVector = std::vector<uint8_t>(jsonToSend.begin(), jsonToSend.end());
  Azure::Core::IO::MemoryBodyStream stream(encodedVector);

  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/Tpm"}, &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  std::string returnedBody(TpmDataSerializer::Deserialize(response));
  return Response<TpmAttestationResult>(TpmAttestationResult{returnedBody}, std::move(response));
}

namespace {
std::shared_timed_mutex SharedStateLock;
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
void AttestationClient::RetrieveResponseValidationCollateral(
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
    TokenValidationCertificateResult returnValue;
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

/** @brief Construct a new Attestation Client object
 *
 * @param endpoint The URL address where the client will send the requests to.
 * @param credential The authentication method to use (required for TPM attestation).
 * @param options The options to customize the client behavior.
 */
std::unique_ptr<AttestationClient> AttestationClientFactory::Create(
    std::string const& endpoint,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    AttestationClientOptions options,
    Azure::Core::Context const& context)
{
  std::unique_ptr<AttestationClient> returnValue(
      new AttestationClient(endpoint, credential, options));
  returnValue->RetrieveResponseValidationCollateral(context);
  // Release the client pointer from the unique pointer to let the parent manage it.
  return returnValue;
}

/** @brief Construct a new anonymous Attestation Client object
 *
 * @param endpoint The URL address where the client will send the requests to.
 * @param options The options to customize the client behavior.
 *
 * @note TPM attestation requires an authenticated attestation client.
 */
std::unique_ptr<AttestationClient> AttestationClientFactory::Create(
    std::string const& endpoint,
    AttestationClientOptions options,
    Azure::Core::Context const& context)
{
  return Create(endpoint, nullptr, options, context);
}
