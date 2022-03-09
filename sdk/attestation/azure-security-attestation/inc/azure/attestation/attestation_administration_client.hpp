// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/attestation/attestation_client_models.hpp"
#include "azure/attestation/attestation_client_options.hpp"
#include <azure/core/context.hpp>
#include <azure/core/url.hpp>
#include <string>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  class HttpPipeline;
}}}} // namespace Azure::Core::Http::_internal

namespace Azure { namespace Security { namespace Attestation {

  /**
   *
   * @brief The AttestationAdministrationClient implements the functionality required by the
   * "Administration" family of attestation service APIs.
   *
   * @note: Attestation administration APIs cannot be used on shared attestation service instances.
   *
   * @details
   * The Administration family of APIs provide APIs to manage:
   *
   * - Attestation policies.
   * - Attestation policy management certificates (Isolated attestation service instances only).
   *
   * There are three flavors of attestation service instances:
   * -# Shared Mode
   * -# AAD Mode
   * -# Isolated Mode
   *
   * Shared mode attestation service instances do not allow any administration actions at all. They
   * exist to allow customers to perform attestation operations without requiring any
   * customizations.
   *
   * AAD Mode instances allow customers to modify attestation policies. When the attestation
   * instance is in AAD mode, the creator of the instance indicates that they trust ARM RBAC and
   * Microsoft AAD to validate client connections to the service. As such, additional proof of
   * authorization is not required for administrative operations.
   *
   */
  class AttestationAdministrationClient final {

  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~AttestationAdministrationClient() = default;

    /**
     * @brief Construct a new Attestation Administration Client object.
     *
     * @param endpoint The URL address where the client will send the requests to.
     * @param credential The authentication token to use.
     * @param options The options to customize the client behavior.
     */
    explicit AttestationAdministrationClient(
        std::string const& endpoint,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        AttestationAdministrationClientOptions const& options
        = AttestationAdministrationClientOptions());

    /**
     * @brief Returns the API version the client was configured with.
     *
     * @returns The API version used when communicating with the attestation service.
     */
    std::string const& ClientVersion() const { return m_apiVersion; }

    /**
     * @brief Construct a new Attestation Administration Client object from another attestation
     * administration client.
     *
     * @param attestationClient An existing attestation client.
     */
    explicit AttestationAdministrationClient(
        AttestationAdministrationClient const& attestationClient)
        : m_endpoint(attestationClient.m_endpoint), m_apiVersion(attestationClient.m_apiVersion),
          m_pipeline(attestationClient.m_pipeline),
          m_tokenValidationOptions(attestationClient.m_tokenValidationOptions){};

    /**
     * @brief Retrieves an Attestation Policy from the service.
     *
     * @param attestationType Attestation type to be used when retrieving the policy.
     * @param options Options to be used when retrieving the policy.
     * @param context User defined context for the operation.
     * @return Response<Models::AttestationToken<std::string>> The returned policy from the service.
     */
    Response<Models::AttestationToken<std::string>> GetAttestationPolicy(
        Models::AttestationType const& attestationType,
        GetPolicyOptions const& options = GetPolicyOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

    /**
     * @brief Sets the attestation policy for the specified AttestationType.
     *
     * @param attestationType Sets the policy on the specified AttestationType.
     * @param policyToSet The policy document to set.
     * @param options Options used when setting the policy, including signer.
     * @param context User defined context for the operation.
     * @return Response<Models::AttestationToken<Models::PolicyResult>> The result of the set policy
     * operation.
     */
    Response<Models::AttestationToken<Models::PolicyResult>> SetAttestationPolicy(
        Models::AttestationType const& attestationType,
        std::string const& policyToSet,
        SetPolicyOptions const& options = SetPolicyOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

    /**
     * @brief Resets the attestation policy for the specified AttestationType to its default.
     *
     * @param attestationType Sets the policy on the specified AttestationType.
     * @param options Options used when setting the policy, including signer.
     * @param context User defined context for the operation.
     * @return Response<Models::AttestationToken<Models::PolicyResult>> The result of the reset
     * policy operation.
     */
    Response<Models::AttestationToken<Models::PolicyResult>> ResetAttestationPolicy(
        Models::AttestationType const& attestationType,
        SetPolicyOptions const& options = SetPolicyOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

    /**
     * @brief Returns an Attestation Token object which would be sent to the attestation service to
     * set an attestation policy.
     *
     * @details
     * To verify that the attestation service received the attestation policy, the service returns
     * the SHA256 hash of the policy token which was sent ot the service. To simplify the customer
     * experience of interacting with the SetPolicy APIs, CreateSetAttestationPolicyToken API will
     * generate the same token that would be send to the service.
     *
     * To ensure that the token which was sent from the client matches the token which was received
     * by the attestation service, the customer can call CreateSetAttestationPolicyToken and then
     * generate the SHA256 of that token and compare it with the value returned by the service - the
     * two hash values should be identical.
     *
     * @param policyToSet The policy document to set.
     * @param signingKey Optional Attestation Signing Key to be used to sign the policy.
     * @return Models::AttestationToken<std::nullptr_t> Attestation token which would be sent to the
     * attestation service based on this signing key.
     *
     * @note: If policyToSet is null, then this generates a policy reset token.
     */
    Models::AttestationToken<std::nullptr_t> CreateSetAttestationPolicyToken(
        Azure::Nullable<std::string> const& policyToSet,
        Azure::Nullable<AttestationSigningKey> const& signingKey = {}) const;

  private:
    Azure::Core::Url m_endpoint;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential const> m_credentials;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    AttestationTokenValidationOptions m_tokenValidationOptions;

    mutable std::vector<Models::AttestationSigner> m_attestationSigners;

    std::vector<Models::AttestationSigner> const& GetAttestationSigners(
        Azure::Core::Context const& context) const;
  };

}}} // namespace Azure::Security::Attestation
