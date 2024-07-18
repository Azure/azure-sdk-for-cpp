// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once
#include "azure/keyvault/administration/rest_client_models.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <string>
#include <thread>

using namespace Azure::Security::KeyVault::Administration::Models;

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
  class BackupRestoreClient;

  /**
   * @brief BackupRestoreOperation : The backup / restore long running operation.
   * @remark Used to handle  both backup and restore operations due to the similarity in patterns
   * and return values.
   */
  class BackupRestoreOperation final : public Azure::Core::Operation<BackupRestoreOperationStatus> {
  private:
    /* BackupRestoreOperation can be constructed only by friends classes (internal
     * creation). The constructor is private and requires internal components.*/
    friend class Azure::Security::KeyVault::Administration::BackupRestoreClient;

    std::shared_ptr<BackupRestoreClient> m_backupRestoreClient;
    BackupRestoreOperationStatus m_value;
    std::string m_continuationToken;
    bool m_isBackupOperation = true;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    Azure::Response<BackupRestoreOperationStatus> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    /**
     * @brief Only friend classes are permitted to construct a RecoverDeletedKeyOperation. This is
     * because a KeyVaultPipelne is required and it is not exposed to customers.
     *
     * @param backupRestoreClient A #BackupRestoreClient that is used for getting status updates.
     * @param response A #BackupRestoreOperationStatus object.
     * @param isBackupOperation A boolean indicating if the operation is a backup operation or a
     * restore.
     */
    BackupRestoreOperation(
        std::shared_ptr<BackupRestoreClient> const& backupRestoreClient,
        BackupRestoreOperationStatus const& status,
        bool isBackupOperation)
        : m_backupRestoreClient{backupRestoreClient}, m_value{status},
          m_continuationToken{status.JobId}, m_isBackupOperation{isBackupOperation} {};
    /**
     * @brief Only friend classes are permitted to construct a RecoverDeletedKeyOperation. This is
     * because a KeyVaultPipelne is required and it is not exposed to customers.
     * @param backupRestoreClient A #BackupRestoreClient that is used for getting status updates.
     * @param continuationToken A string that is used to resume the operation.
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    BackupRestoreOperation(
        std::shared_ptr<BackupRestoreClient> const& backupRestoreClient,
        std::string const& continuationToken,
        bool isBackupOperation)
        : m_backupRestoreClient{backupRestoreClient}, m_continuationToken{continuationToken},
          m_isBackupOperation{isBackupOperation} {};

  public:
    /**
     * @brief Get the BackupRestoreOperationStatus object.
     *
     * @remark The status contains the current progress result at the time of the call.
     *
     * @return A BackupRestoreOperationStatus object.
     */
    BackupRestoreOperationStatus Value() const override { return m_value; }

    /**
     * @brief Get the continuation token used for further status inquiries
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #BackupRestoreOperation from the \p resumeToken fetched from
     * another `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the
     * operation.
     * @param client A #BackupRestoreClient that is used for getting status updates.
     * @param isBackupOperation A boolean indicating if the operation is a backup operation if
     * false it is considered a restore operation.
     * @param context A Azure::Core::Context controlling the request lifetime.
     * @return BackupRestoreOperation
     */
    static BackupRestoreOperation CreateFromResumeToken(
        std::string const& resumeToken,
        BackupRestoreClient const& client,
        bool isBackupOperation,
        Azure::Core::Context const& context = Azure::Core::Context())
    {
      BackupRestoreOperation operation(
          std::make_shared<BackupRestoreClient>(client), resumeToken, isBackupOperation);
      operation.Poll(context);
      return operation;
    }
  };
}}}} // namespace Azure::Security::KeyVault::Administration
