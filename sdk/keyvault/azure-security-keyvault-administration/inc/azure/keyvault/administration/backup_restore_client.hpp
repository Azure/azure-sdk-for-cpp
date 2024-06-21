// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once
#include "azure/keyvault/administration/rest_client_models.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
  using namespace Models;

  class BackupRestoreClient final {
  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~BackupRestoreClient() = default;

    /**
     * @brief Construct a new BackupRestoreClient object
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit BackupRestoreClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        BackupRestoreClientOptions options = BackupRestoreClientOptions());

    /**
     * @brief Construct a new BackupRestoreClient object from another key client.
     *
     * @param backupRestoreClient An existing backup restore client.
     */
    explicit BackupRestoreClient(BackupRestoreClient const& backupRestoreClient) = default;

    /*
     * @brief Creates a full backup using a user-provided SAS token to an Azure blob storage
     * container.
     *
     * @param options Azure blob shared access signature token pointing to a valid Azure blob
     * container where full backup needs to be stored. This token needs to be valid for at least
     * next 24 hours from the time of making this call.
     * @param context The context for the operation can be used for request cancellation.
     * @return A full backup operation.
     */
    Response<FullBackupOperation> FullBackup(
        SasTokenParameter const& azureStorageBlobContainerUri,
        Core::Context const& context = {});

    /*
     * @brief Returns the status of full backup operation.
     *
     * @param jobId Identifier for the full backup operation..
     * @param context The context for the operation can be used for request cancellation.
     * @return A full backup operation.
     */
    Response<FullBackupOperation> FullBackupStatus(
        std::string const& jobId,
        Core::Context const& context = {});

    /*
     * @brief Restores all key materials using the SAS token pointing to a previously stored Azure
     * Blob storage backup folder
     *
     * @param restoreBlobDetails The Azure blob SAS token pointing to a folder where the previous
     * successful full backup was stored.
     * @param context The context for the operation can be used for request cancellation.
     * @return A full restore operation.
     */
    Response<RestoreOperation> FullRestore(
        RestoreOperationParameters const& restoreBlobDetails,
        Core::Context const& context = {});

    /*
     * @brief Returns the status of restore operation.
     *
     * @param jobId Identifier for the restore operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return A restore operation.
     */
    Response<RestoreOperation> RestoreStatus(
        std::string const& jobId,
        Core::Context const& context = {});

    /*
     * @brief  Restores all key versions of a given key using user supplied SAS token pointing to a
     * previously stored Azure Blob storage backup folder.
     *
     * @param keyName The name of the key to be restored from the user supplied backup.
     * @param restoreBlobDetails The Azure blob SAS token pointing to a folder where the previous successful full
       * backup was stored
     * @param context The context for the operation can be used for request cancellation.
     * @return A selective key restore operation.
     */
    Response<SelectiveKeyRestoreOperation> SelectiveKeyRestore(
        std::string const& keyName,
        SelectiveKeyRestoreOperationParameters const& restoreBlobDetails,
        Core::Context const& context = {});

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Core::Url m_vaultBaseUrl;
    std::string m_apiVersion;
  };

}}}} // namespace Azure::Security::KeyVault::Administration
