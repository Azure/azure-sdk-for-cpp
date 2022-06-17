// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/blob_transfer_manager.hpp"

#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"
#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  JobProperties BlobTransferManager::ScheduleUpload(
      const std::string& sourceLocalPath,
      const BlobClient& destinationBlob,
      const ScheduleUploadBlobOptions& options)
  {
    // TODO: verify source and destination
    _internal::JobModel jobModel;
    jobModel.Source = _internal::TransferEnd::CreateFromLocalFile(sourceLocalPath);
    jobModel.Destination = _internal::TransferEnd::CreateFromAzureBlob(destinationBlob);

    _internal::HydrationParameters hydrateOptions;
    hydrateOptions.ErrorHandler = options.ErrorHandler;
    hydrateOptions.ProgressHandler = options.ProgressHandler;
    return m_jobEngine.CreateJob(std::move(jobModel), std::move(hydrateOptions));
  }

  JobProperties BlobTransferManager::ScheduleUploadDirectory(
      const std::string& sourceLocalPath,
      const BlobFolder& destinationBlobFolder,
      const ScheduleUploadBlobOptions& options)
  {
    _internal::JobModel jobModel;
    jobModel.Source = _internal::TransferEnd::CreateFromLocalDirectory(sourceLocalPath);
    jobModel.Destination = _internal::TransferEnd::CreateFromAzureBlobFolder(destinationBlobFolder);

    _internal::HydrationParameters hydrateOptions;
    hydrateOptions.ErrorHandler = options.ErrorHandler;
    hydrateOptions.ProgressHandler = options.ProgressHandler;
    return m_jobEngine.CreateJob(std::move(jobModel), std::move(hydrateOptions));
  }

  JobProperties BlobTransferManager::ScheduleDownload(
      const BlobClient& sourceBlob,
      const std::string& destinationLocalPath,
      const ScheduleDownloadBlobOptions& options)
  {
    // TODO: check destination is dir or file
    _internal::JobModel jobModel;
    jobModel.Source = _internal::TransferEnd::CreateFromAzureBlob(sourceBlob);
    jobModel.Destination = _internal::TransferEnd::CreateFromLocalFile(destinationLocalPath);

    _internal::HydrationParameters hydrateOptions;
    hydrateOptions.ErrorHandler = options.ErrorHandler;
    hydrateOptions.ProgressHandler = options.ProgressHandler;
    return m_jobEngine.CreateJob(std::move(jobModel), std::move(hydrateOptions));
  }

  JobProperties BlobTransferManager::ScheduleDownloadDirectory(
      const BlobFolder& sourceBlobFolder,
      const std::string& destinationLocalPath,
      const ScheduleDownloadBlobOptions& options)
  {
    _internal::JobModel jobModel;
    jobModel.Source = _internal::TransferEnd::CreateFromAzureBlobFolder(sourceBlobFolder);
    jobModel.Destination = _internal::TransferEnd::CreateFromLocalDirectory(destinationLocalPath);

    _internal::HydrationParameters hydrateOptions;
    hydrateOptions.ErrorHandler = options.ErrorHandler;
    hydrateOptions.ProgressHandler = options.ProgressHandler;
    return m_jobEngine.CreateJob(std::move(jobModel), std::move(hydrateOptions));
  }

}}} // namespace Azure::Storage::Blobs
