// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Keys client.
 *
 */

#pragma once

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/certificate_client_options.hpp"

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/core/response.hpp>

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  /**
   * @brief The CertificateClient provides synchronous methods to manage KeyVaultCertificate in
   * Azure Key Vault.
   *
   * @details The client supports retrieving KeyVaultCertificate.
   */
  class CertificateClient
#if !defined(TESTING_BUILD)
      final
#endif
  {
  protected:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    Azure::Core::Url m_vaultUrl;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~CertificateClient() = default;

    /**
     * @brief Construct a new Key Client object
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit CertificateClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CertificateClientOptions options = CertificateClientOptions());

    /**
     * @brief Construct a new Key Client object from another key client.
     *
     * @param keyClient An existing key vault key client.
     */
    explicit CertificateClient(CertificateClient const& keyClient) = default;

    /**
     * @brief Returns the latest version of the KeyVaultCertificate along with its
     * CertificatePolicy.
     *
     * @remark This operation requires the certificates/get permission.
     *
     * @param name The name of the certificate.
     * @param options Optional parameters for this operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response containing the certificate and policy as a KeyVaultCertificateWithPolicy
     * instance.
     */
    Azure::Response<KeyVaultCertificateWithPolicy> GetCertificate(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;
  };
}}}} // namespace Azure::Security::KeyVault::Certificates
