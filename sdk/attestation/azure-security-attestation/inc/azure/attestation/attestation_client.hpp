// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <azure/core/internal/json/json.hpp>

#include <azure/attestation/attestation_client_models.hpp>
#include "attestation_client_options.hpp"
#include <azure/core/internal/http/pipeline.hpp>

namespace Azure { namespace Security { namespace Attestation {

    using namespace Azure::Core;
  class AttestationClient final {
  private:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    Azure::Core::Url m_endpoint;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~AttestationClient() = default;

    /**
     * @brief Construct a new Attestation Client object
     *
     * @param endpoint The URL address where the client will send the requests to.
     * @param credential OPTIONAL The authentication method to use (required for TPM attestation).
     * @param options The options to customize the client behavior.
     */
    explicit AttestationClient(
        std::string const& endpoint,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        AttestationClientOptions options = AttestationClientOptions());

    explicit AttestationClient(
        std::string const& endpoint,
        AttestationClientOptions options = AttestationClientOptions())
        : AttestationClient(endpoint, nullptr, options)
    {
    }

    std::string ClientVersion() const;
    /**
     * @brief Construct a new Key Client object from another key client.
     *
     * @param keyClient An existing key vault key client.
     */
    explicit AttestationClient(AttestationClient const& attestationClient) = default;

    /**
     * @brief Retrieve the metadata signing certificates for this attestation instance.
     * 
     * @returns Attestation Metadata.
     */
    Response<AttestationOpenIdMetadata> GetOpenIdMetadata(
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext);

    /**
     * @brief Retrieve the attestation signing certificates for this attestation instance.
     *
     * @returns Attestation Metadata.
     */
    Response<std::vector<AttestationSigner>> GetAttestationSigningCertificates(
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext);

    /**
    * @brief Attest an SGX enclave, returning an attestation token representing the result
    * of the attestation operation.
    */
    Response<AttestationToken<AttestationResult>> AttestSgxEnclave(
        std::vector<uint8_t> const &sgxQuoteToAttest, 
        AttestOptions options = AttestOptions(),
        Azure::Core::Context const& context
        = Azure::Core::Context::ApplicationContext);

    /**
     * @brief Attest an OpenEnclave report, returning an attestation token representing the result
     * of the attestation operation.
     */
    Response<AttestationToken<AttestationResult>> AttestOpenEnclave(
        std::vector<uint8_t> const& openEnclaveReportToAttest,
        AttestOptions options = AttestOptions(),
        Azure::Core::Context const& context = Azure::Core::Context::ApplicationContext);
  };

}}} // namespace Azure::Security::Attestation
