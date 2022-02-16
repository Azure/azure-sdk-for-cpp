// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_client.hpp"
#include "private/attestation_client_models_private.hpp"
#include "private/attestation_client_private.hpp"
#include "private/attestation_common_request.hpp"
#include "private/attestation_deserializers_private.hpp"
#include "private/package_version.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <shared_mutex>

#include <string>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Security::Attestation::_detail;
using namespace Azure::Security::Attestation::Models::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::_internal;

AttestationClient::AttestationClient(
    std::string const& endpoint,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    AttestationClientOptions options)
    : m_endpoint(endpoint), m_tokenValidationOptions(options.TokenValidationOptions)
{
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  if (credential)
  {
    //      Azure::Core::Credentials::TokenRequestContext const tokenContext
    //          = {{_internal::UrlScope::GetScopeFromUrl(m_vaultUrl)}};

    //      perRetrypolicies.emplace_back(
    //          std::make_unique<BearerTokenAuthenticationPolicy>(credential,
    //          std::move(tokenContext)));
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

Azure::Response<AttestationOpenIdMetadata> AttestationClient::GetOpenIdMetadata(
    Azure::Core::Context const& context) const
{
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, HttpMethod::Get, {".well-known/openid-configuration"}, nullptr);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  auto openIdMetadata(OpenIdMetadataSerializer::Deserialize(response));
  return Response<AttestationOpenIdMetadata>(std::move(openIdMetadata), std::move(response));
}

Azure::Response<AttestationSigningCertificateResult>
AttestationClient::GetAttestationSigningCertificates(Azure::Core::Context const& context) const
{
  auto request
      = AttestationCommonRequest::CreateRequest(m_endpoint, HttpMethod::Get, {"certs"}, nullptr);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
  AttestationSigningCertificateResult returnValue;
  for (const auto& jwk : jsonWebKeySet.Keys)
  {
    AttestationSignerInternal internalSigner(jwk);
    returnValue.Signers.push_back(internalSigner);
  }
  return Response<AttestationSigningCertificateResult>(returnValue, std::move(response));
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestSgxEnclave(
    std::vector<uint8_t> const& sgxQuote,
    AttestOptions options,
    Azure::Core::Context const& context) const
{
  AttestSgxEnclaveRequest attestRequest{
      sgxQuote,
      options.InittimeData,
      options.RuntimeData,
      options.DraftPolicyForAttestation,
      options.Nonce};

  std::string serializedRequest(AttestSgxEnclaveRequestSerializer::Serialize(attestRequest));

  auto encodedVector = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
  Azure::Core::IO::MemoryBodyStream stream(encodedVector);
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/SgxEnclave"}, &stream);

  // Send the request to the service.
  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);

  // Deserialize the Service response token and return the JSON web token returned by the service.
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);

  // Parse the JWT returned by the attestation service.
  auto token
      = AttestationTokenInternal<AttestationResult, AttestationResultSerializer>(responseToken);

  // Validate the token returned by the service. Use the cached attestation signers in the
  // validation.
  std::vector<AttestationSigner> const& signers = GetAttestationSigners(context);
  token.ValidateToken(
      options.TokenValidationOptions ? options.TokenValidationOptions.Value()
                                     : this->m_tokenValidationOptions,
      signers);

  // And return the attestation result to the caller.
  auto returnedToken = AttestationToken<AttestationResult>(token);
  return Response<AttestationToken<AttestationResult>>(returnedToken, std::move(response));
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestOpenEnclave(
    std::vector<uint8_t> const& openEnclaveReport,
    AttestOptions options,
    Azure::Core::Context const& context) const
{
  AttestOpenEnclaveRequest attestRequest{
      openEnclaveReport,
      options.InittimeData,
      options.RuntimeData,
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

  std::vector<AttestationSigner> const& signers = GetAttestationSigners(context);
  token.ValidateToken(
      options.TokenValidationOptions ? options.TokenValidationOptions.Value()
                                     : this->m_tokenValidationOptions,
      signers);

  return Response<AttestationToken<AttestationResult>>(token, std::move(response));
}

namespace {
std::shared_timed_mutex SharedStateLock;
}

std::vector<AttestationSigner> const& AttestationClient::GetAttestationSigners(
    Azure::Core::Context const& context) const
{
  std::unique_lock<std::shared_timed_mutex> stateLock(SharedStateLock);

  if (m_attestationSigners.size() == 0)
  {
    m_attestationSigners = GetAttestationSigningCertificates(context).Value.Signers;
  }
  return m_attestationSigners;
}
