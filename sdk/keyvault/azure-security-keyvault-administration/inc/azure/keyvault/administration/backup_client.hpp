// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once
#include "azure/keyvault/administration/backup_operation.hpp"
#include "azure/keyvault/administration/rest_client_models.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>

#include <memory>
#include <string>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
  /**
   * @brief Define the options to create an SDK Keys client.
   *
   */
  struct BackupClientOptions final : public Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Service Version used.
     *
     */
    std::string ApiVersion{"7.5"};
  };

  /**
   * @brief Backup restore client.
   *
   */
  class BackupClient final {
  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~BackupClient() = default;

    /**
     * @brief Construct a new BackupClient object
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit BackupClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        BackupClientOptions options = BackupClientOptions());

    /**
     * @brief Creates a full backup using a user-provided SAS token to an Azure blob storage
     *        container.
     *
     * @param blobContainerUrl The URL for the blob storage resource.
     * @param sasToken Azure blob shared access signature token pointing to a
     * valid Azure blob container where full backup needs to be
     * stored. This token needs to be valid for at least next 24
     * hours from the time of making this call.
     * @param context The context for the operation can be used for request cancellation.
     * @return A backup restore operation.
     */
    Response<BackupOperation> FullBackup(
        Azure::Core::Url const& blobContainerUrl,
        Models::SasTokenParameter const& sasToken,
        Core::Context const& context = {});

    /**
     * @brief Returns the status of full backup operation.
     *
     * @param jobId Identifier for the full backup operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return Backup restore operation status.
     */
    Response<Models::BackupOperationStatus> FullBackupStatus(
        std::string const& jobId = "",
        Core::Context const& context = {});

    /**
     * @brief Restores all key materials using the SAS token pointing to a previously stored Azure
     *        Blob storage backup folder.
     *
     * @param blobContainerUrl The URL for the blob storage resource, including the path to the blob
     * @param folderToRestore The path to the blob container where the backup resides.
     * @param sasToken Azure blob shared access signature token pointing to a valid Azure blob
     * container where full backup needs to be stored. This token needs to be valid for at least
     * next 24 hours from the time of making this call.
     * @param context The context for the operation can be used for request cancellation.
     * @return A backup restore operation.
     */
    Response<BackupOperation> FullRestore(
        Azure::Core::Url const& blobContainerUrl,
        std::string folderToRestore,
        Models::SasTokenParameter const& sasToken,
        Core::Context const& context = {});

    /**
     * @brief Returns the status of restore operation.
     *
     * @param jobId Identifier for the restore operation.
     * @param context The context for the operation can be used for request cancellation.
     * @return A backup restore operation status.
     */
    Response<Models::BackupOperationStatus> RestoreStatus(
        std::string const& jobId = "",
        Core::Context const& context = {});

    /**
     * @brief  Restores all key versions of a given key using user supplied SAS token pointing to a
     *         previously stored Azure Blob storage backup folder.
     *
     * @param keyName The name of the key to be restored from the user supplied backup.
     * @param blobContainerUrl The URL for the blob storage resource, including the path to the blob
     * @param folderToRestore The path to the blob container where the backup resides.
     * @param sasToken Azure blob shared access signature token pointing to a valid Azure blob
     * container where full backup needs to be stored. This token needs to be valid for at least
     * next 24 hours from the time of making this call.
     * @param context The context for the operation can be used for request cancellation.
     * @return A backup restore operation.
     */
    Response<BackupOperation> SelectiveKeyRestore(
        std::string const& keyName,
        Azure::Core::Url const& blobContainerUrl,
        std::string folderToRestore,
        Models::SasTokenParameter const& sasToken,
        Core::Context const& context = {});

  private:
    std::shared_ptr<Core::Http::_internal::HttpPipeline> m_pipeline;
    Azure::Core::Url m_vaultBaseUrl;
    std::string m_apiVersion;
    Models::KeyVaultServiceError DeserializeKeyVaultServiceError(
        Azure::Core::Json::_internal::json errorFragment);
    Models::BackupOperationStatus DeserializeBackupOperationStatus(
        Azure::Core::Http::RawResponse const& rawResponse);
  };

}}}} // namespace Azure::Security::KeyVault::Administration
