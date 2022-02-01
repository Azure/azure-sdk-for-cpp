// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_client.hpp"

#include "private/package_version.hpp"
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/base64.hpp>
#include "private/attestation_common_request.hpp"
#include "private/attestation_deserializer.hpp"
#include "private/attestation_client_models_private.hpp"

#include <string>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::_detail;
using namespace Azure::Security::Attestation::Models::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::_internal;

std::string AttestationClient::ClientVersion() const { return PackageVersion::ToString(); }

AttestationClient::AttestationClient(
    std::string const& endpoint,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    AttestationClientOptions options)
    : m_endpoint(endpoint)
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
    Azure::Core::Context const& context)
{
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, HttpMethod::Get, {".well-known/openid-configuration"}, nullptr);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  auto openIdMetadata(OpenIdMetadataDeserializer::Deserialize(response));
  return Response<AttestationOpenIdMetadata>(std::move(openIdMetadata), std::move(response));
}

Azure::Response<std::vector<AttestationSigner>> AttestationClient::GetAttestationSigningCertificates(
    Azure::Core::Context const& context)
{
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, HttpMethod::Get, {"certs"}, nullptr);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
  std::vector<AttestationSigner> signers;
  for (const auto &jwk : jsonWebKeySet.Keys)
  {
    AttestationSignerInternal internalSigner(jwk);
    signers.push_back(internalSigner);
  }
  return Response<std::vector<AttestationSigner>>(signers, std::move(response));
}


Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestSgxEnclave(
    std::vector<uint8_t> const& sgxQuote,
    AttestOptions options,
    Azure::Core::Context const& context)
{
  AttestSgxEnclaveRequest attestRequest{
      sgxQuote,
      {options.InittimeData.Data, options.InittimeData.DataType.ToString()},
      {options.RuntimeData.Data, options.RuntimeData.DataType.ToString()},
      options.DraftPolicyForAttestation};
  std::string serializedRequest(AttestSgxEnclaveRequestSerializer::Serialize(attestRequest));

  auto encodedVector = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
  Azure::Core::IO::MemoryBodyStream stream(encodedVector);
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/SgxEnclave"}, &stream);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);
  auto token = AttestationTokenInternal<AttestationResult, AttestationResultDeserializer>(responseToken, response);
  auto returnedToken = AttestationToken<AttestationResult>(token);
  return Response<AttestationToken<AttestationResult>>(returnedToken, std::move(response));
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestOpenEnclave(
    std::vector<uint8_t> const& openEnclaveReport,
    AttestOptions options,
    Azure::Core::Context const& context)
{
  AttestOpenEnclaveRequest attestRequest{
      openEnclaveReport,
      {options.InittimeData.Data, options.InittimeData.DataType.ToString()},
      {options.RuntimeData.Data, options.RuntimeData.DataType.ToString()},
      options.DraftPolicyForAttestation};
  std::string serializedRequest(AttestOpenEnclaveRequestSerializer::Serialize(attestRequest));

  auto encodedVector = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
  Azure::Core::IO::MemoryBodyStream stream(encodedVector);
  auto request = AttestationCommonRequest::CreateRequest(
      m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/OpenEnclave"}, &stream);

  auto response = AttestationCommonRequest::SendRequest(*m_pipeline, request, context);
  std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);
  auto token = AttestationTokenInternal<AttestationResult, AttestationResultDeserializer>(
      responseToken, response);
  return Response<AttestationToken<AttestationResult>>(token, std::move(response));
}
