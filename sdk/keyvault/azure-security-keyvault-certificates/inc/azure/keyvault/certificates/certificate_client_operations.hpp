
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Certificate long running operations.
 *
 */

#pragma once

#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "azure/keyvault/certificates/dll_import_export.hpp"

#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  class CertificateClient;
  /**
   * @brief Represents a create certificate long running operation
   */
  class CreateCertificateOperation final
      : public Azure::Core::Operation<CertificateOperationProperties> {

    friend class CertificateClient;

  private:
    std::shared_ptr<CertificateClient> m_certificateClient;
    CertificateOperationProperties m_value;
    std::string m_continuationToken;

    Azure::Response<CertificateOperationProperties> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    /*
     * Only friend classes are permitted to call the constructor .
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    CreateCertificateOperation(
        std::shared_ptr<CertificateClient> certificateClient,
        Azure::Response<CertificateOperationProperties> response);

    CreateCertificateOperation(
        std::string resumeToken,
        std::shared_ptr<CertificateClient> certificateClient);

    /**
     * @brief Get the #Azure::Core::Http::RawResponse of the operation request.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Certificates::CertificateOperationProperties
     * object.
     *
     * @return A CertificateOperationProperties object.
     */
    CertificateOperationProperties Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the
     * operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #CreateCertificateOperation from the \p resumeToken fetched from another
     * `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the
     * operation.
     * @param client A #CertificateClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return CreateCertificateOperation
     */
    static CreateCertificateOperation CreateFromResumeToken(
        std::string const& resumeToken,
        CertificateClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());

    /**
     * @brief Cancels the operation.
     *
     * @param context A #Azure::Core::Context controlling the request lifetime.
     */
    void Cancel(Azure::Core::Context const& context = Azure::Core::Context());

    /**
     * @brief Delete operation.
     *
     * @param context A #Azure::Core::Context controlling the request lifetime.
     */
    void Delete(Azure::Core::Context const& context = Azure::Core::Context());

    /**
     * @brief Determines if the operation has completed.
     *
     * @return Completed status.
     */
    bool IsCompleted() const;
  };

  /**
   * @brief Represents a delete certificate long running operation
   */
  class DeleteCertificateOperation final : public Azure::Core::Operation<DeletedCertificate> {

    friend class CertificateClient;

  private:
    std::shared_ptr<CertificateClient> m_certificateClient;
    DeletedCertificate m_value;
    std::string m_continuationToken;

    Azure::Response<DeletedCertificate> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    /*
     * Only friend classes are permitted to call the constructor .
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    DeleteCertificateOperation(
        std::shared_ptr<CertificateClient> certificateClient,
        Azure::Response<DeletedCertificate> response);

    DeleteCertificateOperation(
        std::string resumeToken,
        std::shared_ptr<CertificateClient> certificateClient);

    /**
     * @brief Get the #Azure::Core::Http::RawResponse of the operation request.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Certificates::DeletedCertificate object.
     *
     * @return A deleted certificate object.
     */
    DeletedCertificate Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the
     * operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #DeleteCertificateOperation from the \p resumeToken fetched from another
     * `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the
     * operation.
     * @param client A #CertificateClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return DeleteCertificateOperation
     */
    static DeleteCertificateOperation CreateFromResumeToken(
        std::string const& resumeToken,
        CertificateClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());
  };

  /**
   * @brief Represents a recover deleted certificate long running operation
   */
  class RecoverDeletedCertificateOperation final
      : public Azure::Core::Operation<KeyVaultCertificateWithPolicy> {

    friend class CertificateClient;

  private:
    std::shared_ptr<CertificateClient> m_certificateClient;
    KeyVaultCertificateWithPolicy m_value;
    std::string m_continuationToken;

    Azure::Response<KeyVaultCertificateWithPolicy> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    /*
     * Only friend classes are permitted to call the constructor .
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    RecoverDeletedCertificateOperation(
        std::shared_ptr<CertificateClient> certificateClient,
        Azure::Response<KeyVaultCertificateWithPolicy> response);

    RecoverDeletedCertificateOperation(
        std::string resumeToken,
        std::shared_ptr<CertificateClient> certificateClient);

    /**
     * @brief Get the #Azure::Core::Http::RawResponse of the operation request.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Certificates::KeyVaultCertificateWithPolicy
     * object.
     *
     * @return A key vault certificate object.
     */
    KeyVaultCertificateWithPolicy Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the
     * operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #RecoverDeletedCertificateOperation from the \p resumeToken fetched from
     * another `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the
     * operation.
     * @param client A #CertificateClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return RecoverDeletedCertificateOperation
     */
    static RecoverDeletedCertificateOperation CreateFromResumeToken(
        std::string const& resumeToken,
        CertificateClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());
  };
}}}} // namespace Azure::Security::KeyVault::Certificates
