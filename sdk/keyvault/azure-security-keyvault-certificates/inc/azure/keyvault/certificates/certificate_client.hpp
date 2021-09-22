// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Certificates client.
 *
 */

#pragma once

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/certificate_client_operations.hpp"
#include "azure/keyvault/certificates/certificate_client_options.hpp"
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>

#include <memory>
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
     * @param context The context for the operation can be used for request cancellation.
     * @return A response containing the certificate and policy as a KeyVaultCertificateWithPolicy
     * instance.
     */
    Azure::Response<KeyVaultCertificateWithPolicy> GetCertificate(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Returns a specific version of the certificate without its CertificatePolicy.
     *
     * @details If the version is not set in the options, the latest version is returned.
     *
     * @remark This operation requires the certificates/get permission.
     *
     * @param name The name of the certificate.
     * @param options Optional parameters for this operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response containing the certificate and policy as a KeyVaultCertificateWithPolicy
     * instance.
     */
    Azure::Response<KeyVaultCertificate> GetCertificateVersion(
        std::string const& name,
        GetCertificateOptions const& options = GetCertificateOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a new certificate.
     *
     * @details If this is the first version, the certificate resource is created.
     *
     * @remark This operation requires the certificates/create permission.
     *
     * @param name The name of the certificate.
     * @param parameters Parameters for this operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return CreateCertificateOperation instance used to determine create status.
     */
    CreateCertificateOperation StartCreateCertificate(
        std::string const& name,
        CertificateCreateParameters const& parameters,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Retrieves information about the specified deleted certificate.
     *
     * @details The GetDeletedCertificate operation retrieves the deleted certificate
     * information plus its attributes, such as retention interval,
     * scheduled permanent deletion and the current deletion recovery level.
     *
     * @remark This operation requires the certificates/get permission.
     *
     * @param name The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return The deleted certificate.
     */
    Azure::Response<DeletedCertificate> GetDeletedCertificate(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Permanently deletes the specified deleted certificate.
     *
     * @details The PurgeDeletedCertificate operation performs an irreversible
     * deletion of the specified certificate, without possibility for recovery.
     * The operation is not available if the recovery level does not specify 'Purgeable'
     *
     * @remark This operation requires the certificate/purge permission.
     *
     * @param name The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Empty object.
     */
    Azure::Response<PurgedCertificate> PurgeDeletedCertificate(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes a certificate from a specified key vault.
     *
     * @details Deletes all versions of a certificate object along with its associated policy.
     * Delete certificate cannot be used to remove individual versions of a certificate object.
     *
     * @remark This operation requires the certificate/delete permission.
     *
     * @param name The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Delete Certificate operation.
     */
    DeleteCertificateOperation StartDeleteCertificate(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Recovers the deleted certificate back to its current version under /certificates.
     *
     * @details The StartRecoverDeletedCertificate operation performs the reversal of the Delete
     * operation. The operation is applicable in vaults enabled for soft-delete, and must be issued
     * during the retention interval (available in the deleted certificate's attributes).
     *
     * @remark This operation requires the certificate/recover permission.
     *
     * @param name The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Recover deleted certificate operation.
     */
    RecoverDeletedCertificateOperation StartRecoverDeletedCertificate(
        std::string const& name,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

  private:
    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) const;

    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path = {},
        Azure::Core::IO::BodyStream* content = nullptr) const;
  };
}}}} // namespace Azure::Security::KeyVault::Certificates
