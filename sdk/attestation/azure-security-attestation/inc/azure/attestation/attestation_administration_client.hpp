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

  class AttestationClient;

  /**
   *
   * The AttestationAdministrationClient implements the functionality required by the
   * "Administration" family of attestation service APIs.
   *
   * @note: Attestation administration APIs cannot be used on shared attestation service instances.
   *
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
   * Microsoft AAD to validate client connections to the service. As such, additional authentication
   * is not required for administrative operations.
   *
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
     * @brief Construct a new Attestation Administration Client object
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

    /** @brief Creates a new {@link AttestationClient} with the same URL and HTTP
     * pipeline as this AttestationAdministrationClient.
     *
     * @return A new {@link AttestationClient} instance..
     */
    AttestationClient AsAttestationClient() const;

    /** @brief Retrieves the attestation policy for the specified attestation type.
     */

    Response<Models::AttestationToken<std::string>> GetAttestationPolicy(
        Models::AttestationType const& attestationType,
        GetPolicyOptions const& options = GetPolicyOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext) const;

  protected:
    Azure::Core::Url m_endpoint;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential const> m_credentials;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    AttestationTokenValidationOptions m_tokenValidationOptions;

    mutable std::vector<Models::AttestationSigner> m_attestationSigners;

    std::vector<Models::AttestationSigner> const& GetAttestationSigners(
        Azure::Core::Context const& context) const;

  private:
    friend class AttestationClient;
  };

}}} // namespace Azure::Security::Attestation
