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
#if defined(TESTING_BUILD)
  namespace Test {
    class KeyVaultCertificateClientTest;
  }
#endif
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
    friend class CreateCertificateOperation;

#if defined(TESTING_BUILD)
    friend class Test::KeyVaultCertificateClientTest;
#endif

  private:
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
     * @brief Construct a new Certificate Client object from another certificate client.
     *
     * @param certificateClient An existing key vault certificate client.
     */
    explicit CertificateClient(CertificateClient const& certificateClient) = default;

    /**
     * @brief Return the latest version of the KeyVaultCertificate along with its
     * CertificatePolicy.
     *
     * @remark This operation requires the certificates/get permission.
     *
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response containing the certificate and policy as a KeyVaultCertificateWithPolicy
     * instance.
     */
    Azure::Response<KeyVaultCertificateWithPolicy> GetCertificate(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Return a specific version of the certificate without its CertificatePolicy.
     *
     * @details If the version is not set in the options, the latest version is returned.
     *
     * @remark This operation requires the certificates/get permission.
     *
     * @param certificateName The name of the certificate.
     * @param certificateVersion The version of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response containing the certificate as a KeyVaultCertificate
     * instance.
     */
    Azure::Response<KeyVaultCertificate> GetCertificateVersion(
        std::string const& certificateName,
        std::string const& certificateVersion,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Create a new certificate.
     *
     * @details If this is the first version, the certificate resource is created.
     *
     * @remark This operation requires the certificates/create permission.
     *
     * @param certificateName The name of the certificate.
     * @param options Options for this operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return CreateCertificateOperation instance used to determine create status.
     */
    CreateCertificateOperation StartCreateCertificate(
        std::string const& certificateName,
        CertificateCreateOptions const& options,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Create a new certificate issuer.
     *
     * @details The  operation adds or updates the specified certificate issuer.
     *
     * @remark This operation requires the certificates/setissuers permission.
     *
     * @param issuerName The name of the issuer.
     * @param certificateIssuer The certificate issuer.
     * @param context The context for the operation can be used for request cancellation.
     * @return CertificateIssuer instance used to determine create status.
     */
    Azure::Response<CertificateIssuer> CreateIssuer(
        std::string const& issuerName,
        CertificateIssuer const& certificateIssuer,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List the specified certificate issuer.
     *
     * @details The GetCertificateIssuer operation returns the specified
     * certificate issuer resources in the specified key vault.
     *
     * @remark This operation requires the certificates/manageissuers/getissuers permission.
     *
     * @param issuerName The certificate issuer name.
     * @param context The context for the operation can be used for request cancellation.
     * @return CertificateIssuer instance.
     */
    Azure::Response<CertificateIssuer> GetIssuer(
        std::string const& issuerName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Update the specified certificate issuer.
     *
     * @details The operation performs an update on the specified certificate issuer entity.
     *
     * @remark This operation requires the certificates/setissuers permission.
     *
     * @param issuerName The name of the issuer.
     * @param certificateIssuer The certificate issuer.
     * @param context The context for the operation can be used for request cancellation.
     * @return CertificateIssuer instance.
     */
    Azure::Response<CertificateIssuer> UpdateIssuer(
        std::string const& issuerName,
        CertificateIssuer const& certificateIssuer,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Delete the specified certificate issuer.
     *
     * @details The operation permanently removes the specified certificate issuer from the vault.
     *
     * @remark This operation requires the certificates/manageissuers/deleteissuers permission.
     *
     * @param issuerName The certificate issuer name.
     * @param context The context for the operation can be used for request cancellation.
     * @return CertificateIssuer instance.
     */
    Azure::Response<CertificateIssuer> DeleteIssuer(
        std::string const& issuerName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List the certificate contacts for a specified key vault.
     *
     * @details The GetContacts operation returns the set of certificate contact
     * resources in the specified key vault.
     *
     * @remark This operation requires the certificates/managecontacts permission.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The contacts list for the key vault certificate.
     */
    Azure::Response<CertificateContactsResult> GetContacts(
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Delete the certificate contacts for a specified key vault.
     *
     * @details Deletes the certificate contacts for a specified key vault certificate.
     *
     * @remark This operation requires the certificates/managecontacts permission.
     *
     * @param context The context for the operation can be used for request cancellation.
     * @return The contacts for the key vault certificate.
     */
    Azure::Response<CertificateContactsResult> DeleteContacts(
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Set certificate contacts.
     *
     * @details Set the certificate contacts for the specified key vault.
     *
     * @remark This operation requires the certificates/managecontacts permission.
     *
     * @param contacts The contacts for the key vault certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return The contacts for the key vault certificate.
     */
    Azure::Response<CertificateContactsResult> SetContacts(
        std::vector<CertificateContact> const& contacts,
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
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return The deleted certificate.
     */
    Azure::Response<DeletedCertificate> GetDeletedCertificate(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Permanently deletes the specified deleted certificate.
     *
     * @details The PurgeDeletedCertificate operation performs an irreversible
     * deletion of the specified certificate, without possibility for recovery.
     * The operation is not available if the recovery level does not specify 'Purgeable'.
     *
     * @remark This operation requires the certificate/purge permission.
     *
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Empty object.
     */
    Azure::Response<PurgedCertificate> PurgeDeletedCertificate(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes a certificate from a specified key vault.
     *
     * @details Deletes all versions of a certificate object along with its associated policy.
     * Delete certificate cannot be used to remove individual versions of a certificate object.
     *
     * @remark This operation requires the certificate/delete permission.
     *
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Delete Certificate operation.
     */
    DeleteCertificateOperation StartDeleteCertificate(
        std::string const& certificateName,
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
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Recover deleted certificate operation.
     */
    RecoverDeletedCertificateOperation StartRecoverDeletedCertificate(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief "List the policy for a certificate.
     *
     * @details The GetCertificatePolicy operation returns the specified certificate policy
     * resources in the specified key vault.
     *
     * @remark This operation requires the certificates/get permission.
     *
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return The contact properties.
     */
    Azure::Response<CertificatePolicy> GetCertificatePolicy(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Update the policy for a certificate.
     *
     * @details Set specified members in the certificate policy. Leave others as null.
     *
     * @remark This operation requires the certificates/update permission.
     *
     * @param certificateName The name of the certificate.
     * @param certificatePolicy The updated certificate policy.
     * @param context The context for the operation can be used for request cancellation.
     * @return The updated contact properties.
     */
    Azure::Response<CertificatePolicy> UpdateCertificatePolicy(
        std::string const& certificateName,
        CertificatePolicy const& certificatePolicy,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Backs up the specified certificate.
     *
     * @details Request that a backup of the specified certificate be downloaded to the client.
     * All versions of the certificate will be downloaded.
     *
     * @remark This operation requires the certificates/backup permission.
     *
     * @param certificateName The name of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return Certificate backup.
     */
    Azure::Response<BackupCertificateResult> BackupCertificate(
        std::string certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Restores a backed up certificate to a vault.
     *
     * @details Restore a backed up certificate, and all its versions, to a vault.
     *
     * @remark This operation requires the certificates/restore permission.
     *
     * @param certificateBackup The backup blob to restore.
     * @param context The context for the operation can be used for request cancellation.
     * @return The restored certificate.
     */
    Azure::Response<KeyVaultCertificateWithPolicy> RestoreCertificateBackup(
        std::vector<uint8_t> const& certificateBackup,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List certificates in a specified key vault.
     *
     * @details The GetPropertiesOfCertificates operation returns
     * the set of certificates resources in the specified key vault.
     *
     * @remark This operation requires the certificates/list permission.
     *
     * @param options The options for the request.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response message containing a list of certificates along with a link to the next
     * page of certificates.
     */
    CertificatePropertiesPagedResponse GetPropertiesOfCertificates(
        GetPropertiesOfCertificatesOptions const& options = GetPropertiesOfCertificatesOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List the versions of a certificate.
     *
     * @details The GetCertificateVersions operation returns the versions
     * of a certificate in the specified key vault.
     *
     * @remark This operation requires the certificates/list permission.
     *
     * @param certificateName The name of the certificate.
     * @param options The options for the request.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response message containing a list of certificate versions along with a link to the
     * next page of certificates.
     */
    CertificatePropertiesPagedResponse GetPropertiesOfCertificateVersions(
        std::string const& certificateName,
        GetPropertiesOfCertificateVersionsOptions const& options
        = GetPropertiesOfCertificateVersionsOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief List certificate issuers for a specified key vault.
     *
     * @details The GetPropertiesOfIssuers operation returns the set of certificate issuer resources
     * in the specified key vault.
     *
     * @remark This operation requires the certificates/manageissuers/getissuers permission.
     *
     * @param options The options for the request.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response message containing a list of issuers along with a link to the
     * next page of certificates.
     */
    IssuerPropertiesPagedResponse GetPropertiesOfIssuers(
        GetPropertiesOfIssuersOptions const& options = GetPropertiesOfIssuersOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Lists the deleted certificates in the specified vault currently available for
     * recovery.
     *
     * @details The GetDeletedCertificates operation retrieves the certificates in the current vault
     * which are in a deleted state and ready for recovery or purging. This operation includes
     * deletion-specific information. This operation requires the certificates/get/list permission.
     *
     * @remark This operation can only be enabled on soft-delete enabled vaults.
     *
     * @param options The options for the request.
     * @param context The context for the operation can be used for request cancellation.
     * @return A response message containing a list of deleted certificates in the vault along with
     * a link to the next page of deleted certificates.
     */
    DeletedCertificatesPagedResponse GetDeletedCertificates(
        GetDeletedCertificatesOptions const& options = GetDeletedCertificatesOptions(),
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Imports a certificate into a specified key vault.
     *
     * @details Imports an existing valid certificate, containing a private key, into Azure Key
     * Vault. This operation requires the certificates/import permission. The certificate to be
     * imported can be in either PFX or PEM format. If the certificate is in PEM format the PEM file
     * must contain the key as well as x509 certificates. Key Vault will only accept a key in PKCS#8
     * format.
     *
     * @remark This operation requires the certificates/import permission.
     *
     * @param certificateName The name of the certificate
     * @param options The options for the request.
     * @param context The context for the operation can be used for request cancellation.
     * @return Imported certificate bundle to the vault.
     */
    Azure::Response<KeyVaultCertificateWithPolicy> ImportCertificate(
        std::string const& certificateName,
        ImportCertificateOptions const& options,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Merges a certificate or a certificate chain with a key pair existing on the server.
     *
     * @details The MergeCertificate operation performs the merging of a certificate or certificate
     * chain with a key pair currently available in the service.
     *
     * @remark This operation requires the certificates/create permission.
     *
     * @param certificateName The name of the certificate.
     * @param options The options for the request.
     * @param context The context for the operation can be used for request cancellation.
     * @return Merged certificate bundle to the vault.
     */
    Azure::Response<KeyVaultCertificateWithPolicy> MergeCertificate(
        std::string const& certificateName,
        MergeCertificateOptions const& options,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    /**
     * @brief Updates the specified attributes associated with a certificate.
     *
     * @details The UpdateCertificate operation applies the specified update on the given
     * certificate; the only elements updated are the certificate's attributes.
     *
     * @remark This operation requires the certificates/update permission.
     *
     * @param certificateName The name of the certificate.
     * @param certificateVersion The version of the certificate.
     * @param certificateProperties The the new properties of the certificate.
     * @param context The context for the operation can be used for request cancellation.
     * @return The updated certificate.
     */
    Azure::Response<KeyVaultCertificate> UpdateCertificateProperties(
        std::string const& certificateName,
        std::string const& certificateVersion,
        CertificateProperties const& certificateProperties,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

  private:
    Azure::Response<CertificateOperationProperties> GetPendingCertificateOperation(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    Azure::Response<CertificateOperationProperties> DeletePendingCertificateOperation(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    Azure::Response<CertificateOperationProperties> CancelPendingCertificateOperation(
        std::string const& certificateName,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) const;

    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path = {},
        Azure::Core::IO::BodyStream* content = nullptr) const;

    Azure::Core::Http::Request ContinuationTokenRequest(
        std::vector<std::string> const& path,
        const Azure::Nullable<std::string>& NextPageToken) const;
  };
}}}} // namespace Azure::Security::KeyVault::Certificates
