// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/attestation/attestation_client.hpp"

#include "private/attestation_client_models_private.hpp"
#include "private/attestation_client_private.hpp"
#include "private/attestation_common_request.hpp"
#include "private/attestation_deserializers_private.hpp"
#include "private/package_version.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <shared_mutex>
#include <string>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace Azure::Security::Attestation::_detail;
using namespace Azure::Security::Attestation::Models::_detail;
using namespace Azure::Core::Tracing::_internal;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

AttestationClient::AttestationClient(
    std::string const& endpoint,
    std::shared_ptr<const Core::Credentials::TokenCredential> credential,
    AttestationClientOptions options)
    : m_endpoint{endpoint}, m_apiVersion{options.ApiVersion},
      m_tokenValidationOptions{options.TokenValidationOptions},
      m_tracingFactory{
          options,
          "Microsoft.Attestation",
          "azure-security-attestation-cpp",
          PackageVersion::ToString()}
{
  std::vector<std::unique_ptr<HttpPolicy>> perRetryPolicies;
  if (credential)
  {
    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes = {"https://attest.azure.net/.default"};

    perRetryPolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }
  std::vector<std::unique_ptr<HttpPolicy>> perCallPolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      "security.attestation",
      PackageVersion::ToString(),
      std::move(perRetryPolicies),
      std::move(perCallPolicies));
}

Azure::Response<OpenIdMetadata> AttestationClient::GetOpenIdMetadata(
    Azure::Core::Context const& context) const
{
  auto tracingContext(m_tracingFactory.CreateTracingContext("GetOpenIdMetadata", context));
  try
  {
    auto request = AttestationCommonRequest::CreateRequest(
        m_endpoint, HttpMethod::Get, {".well-known/openid-configuration"}, nullptr);

    auto response
        = AttestationCommonRequest::SendRequest(*m_pipeline, request, tracingContext.Context);
    auto openIdMetadata(OpenIdMetadataSerializer::Deserialize(response));

    return Response<OpenIdMetadata>(std::move(openIdMetadata), std::move(response));
  }
  catch (std::runtime_error const& ex)
  {
    tracingContext.Span.AddEvent(ex);
    throw;
  }
}

Azure::Response<TokenValidationCertificateResult> AttestationClient::GetTokenValidationCertificates(
    Azure::Core::Context const& context) const
{
  auto tracingContext(
      m_tracingFactory.CreateTracingContext("GetTokenValidationCertificates", context));
  try
  {

    auto request
        = AttestationCommonRequest::CreateRequest(m_endpoint, HttpMethod::Get, {"certs"}, nullptr);

    auto response
        = AttestationCommonRequest::SendRequest(*m_pipeline, request, tracingContext.Context);
    auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
    TokenValidationCertificateResult returnValue;
    for (const auto& jwk : jsonWebKeySet.Keys)
    {
      AttestationSignerInternal internalSigner(jwk);
      returnValue.Signers.push_back(internalSigner);
    }
    return Response<TokenValidationCertificateResult>(returnValue, std::move(response));
  }
  catch (std::runtime_error const& ex)
  {
    tracingContext.Span.AddEvent(ex);
    throw;
  }
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestSgxEnclave(
    std::vector<uint8_t> const& sgxQuote,
    AttestSgxEnclaveOptions options,
    Azure::Core::Context const& context) const
{
  auto tracingContext(m_tracingFactory.CreateTracingContext("AttestSgxEnclave", context));
  try
  {

    AttestSgxEnclaveRequest attestRequest{
        sgxQuote,
        options.InitTimeData,
        options.RunTimeData,
        options.DraftPolicyForAttestation,
        options.Nonce};

    const std::string serializedRequest(
        AttestSgxEnclaveRequestSerializer::Serialize(attestRequest));

    const auto encodedVector
        = std::vector<uint8_t>(serializedRequest.begin(), serializedRequest.end());
    Azure::Core::IO::MemoryBodyStream stream(encodedVector);
    auto request = AttestationCommonRequest::CreateRequest(
        m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/SgxEnclave"}, &stream);

    // Send the request to the service.
    auto response
        = AttestationCommonRequest::SendRequest(*m_pipeline, request, tracingContext.Context);

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
  catch (std::runtime_error const& ex)
  {
    tracingContext.Span.AddEvent(ex);
    throw;
  }
}

Azure::Response<AttestationToken<AttestationResult>> AttestationClient::AttestOpenEnclave(
    std::vector<uint8_t> const& openEnclaveReport,
    AttestOpenEnclaveOptions options,
    Azure::Core::Context const& context) const
{
  auto tracingContext(m_tracingFactory.CreateTracingContext("AttestOpenEnclave", context));
  try
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

    auto response
        = AttestationCommonRequest::SendRequest(*m_pipeline, request, tracingContext.Context);
    std::string responseToken = AttestationServiceTokenResponseSerializer::Deserialize(response);
    auto token
        = AttestationTokenInternal<AttestationResult, AttestationResultSerializer>(responseToken);
    token.ValidateToken(
        options.TokenValidationOptionsOverride ? *options.TokenValidationOptionsOverride
                                               : this->m_tokenValidationOptions,
        m_attestationSigners);

    return Response<AttestationToken<AttestationResult>>(token, std::move(response));
  }
  catch (std::runtime_error const& ex)
  {
    tracingContext.Span.AddEvent(ex);
    throw;
  }
}

Azure::Response<TpmAttestationResult> AttestationClient::AttestTpm(
    std::vector<uint8_t> const& dataToAttest,
    AttestTpmOptions const&,
    Azure::Core::Context const& context) const
{
  auto tracingContext(m_tracingFactory.CreateTracingContext("AttestTpm", context));
  try
  {
    std::string jsonToSend = TpmDataSerializer::Serialize(dataToAttest);
    auto encodedVector = std::vector<uint8_t>(jsonToSend.begin(), jsonToSend.end());
    Azure::Core::IO::MemoryBodyStream stream(encodedVector);

    auto request = AttestationCommonRequest::CreateRequest(
        m_endpoint, m_apiVersion, HttpMethod::Post, {"attest/Tpm"}, &stream);

    // Send the request to the service.
    auto response
        = AttestationCommonRequest::SendRequest(*m_pipeline, request, tracingContext.Context);
    std::vector<uint8_t> returnedBody{TpmDataSerializer::Deserialize(response)};
    return Response<TpmAttestationResult>(TpmAttestationResult{returnedBody}, std::move(response));
  }
  catch (std::runtime_error const& ex)
  {
    tracingContext.Span.AddEvent(ex);
    throw;
  }
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
void AttestationClient::RetrieveResponseValidationCollateral(Azure::Core::Context const& context)
{
  auto tracingContext(m_tracingFactory.CreateTracingContext("Create", context));
  try
  {
    std::unique_lock<std::shared_timed_mutex> stateLock(SharedStateLock);

    if (m_attestationSigners.empty())
    {
      stateLock.unlock();
      auto request = AttestationCommonRequest::CreateRequest(
          m_endpoint, HttpMethod::Get, {"certs"}, nullptr);
      auto response
          = AttestationCommonRequest::SendRequest(*m_pipeline, request, tracingContext.Context);
      auto jsonWebKeySet(JsonWebKeySetSerializer::Deserialize(response));
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
      tracingContext.Span.SetStatus(SpanStatus::Ok);
    }
  }
  catch (std::runtime_error const& ex)
  {
    tracingContext.Span.AddEvent(ex);
    throw;
  }
}

Azure::Security::Attestation::AttestationClient AttestationClient::Create(
    std::string const& endpoint,
    std::shared_ptr<const Core::Credentials::TokenCredential> credential,
    AttestationClientOptions const& options,
    Azure::Core::Context const& context)
{
  AttestationClient returnValue(endpoint, credential, options);
  returnValue.RetrieveResponseValidationCollateral(context);
  // Release the client pointer from the unique pointer to let the parent manage it.
  return returnValue;
}

Azure::Security::Attestation::AttestationClient AttestationClient::Create(
    std::string const& endpoint,
    AttestationClientOptions options,
    Azure::Core::Context const& context)
{
  return Create(endpoint, nullptr, options, context);
}
