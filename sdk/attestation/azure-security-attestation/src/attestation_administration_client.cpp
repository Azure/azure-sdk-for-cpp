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

namespace Azure { namespace Security { namespace Attestation {
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

  Response<Models::AttestationToken<std::string>>
  AttestationAdministrationClient::GetAttestationPolicy(
      AttestationType const& attestationType,
      GetPolicyOptions const& options,
      Azure::Core::Context const& context) const
  {
    Log::Write(
        Logger::Level::Informational,
        std::string("Get Policy for AttestationType:") + attestationType.ToString());

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
        options.TokenValidationOptions ? options.TokenValidationOptions.Value()
                                       : this->m_tokenValidationOptions,
        signers);

    // Extract the underlying policy token from the response.
    std::string policyTokenValue
        = static_cast<AttestationToken<Models::_detail::PolicyResult>>(resultToken)
              .Body.PolicyToken.Value();

    // TPM policies are empty by default, at least in our test instances, so handle the empty policy
    // token case.
    auto policyTokenI
        = AttestationTokenInternal<StoredAttestationPolicy, StoredAttestationPolicySerializer>(
            policyTokenValue);
    AttestationToken<StoredAttestationPolicy> policyToken(policyTokenI);
    std::string returnPolicy;
    if (policyToken.Body.AttestationPolicy)
    {
      std::vector<uint8_t> policyUtf8 = policyToken.Body.AttestationPolicy.Value();
      returnPolicy = std::string(policyUtf8.begin(), policyUtf8.end());
    }

    // Construct a token whose body is the policy, but whose token is the response from the
    // service.
    auto returnedToken = AttestationTokenInternal<std::string>(responseToken, returnPolicy);
    return Response<AttestationToken<std::string>>(returnedToken, std::move(response));
  }

  std::vector<AttestationSigner> const& AttestationAdministrationClient::GetAttestationSigners(
      Azure::Core::Context const& context) const
  {
    std::unique_lock<std::shared_timed_mutex> stateLock(SharedStateLock);

    if (m_attestationSigners.size() == 0)
    {
      auto request = AttestationCommonRequest::CreateRequest(
          m_endpoint, HttpMethod::Get, {"certs"}, nullptr);
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
}}} // namespace Azure::Security::Attestation
