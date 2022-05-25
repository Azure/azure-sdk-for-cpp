// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/blob_transfer_manager.hpp"

#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"
#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"
#include "azure/storage/datamovement/tasks/upload_blobs_from_directory_task.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  JobProperties BlobTransferManager::ScheduleUpload(
      const std::string& sourceLocalPath,
      const BlobClient& destinationBlob,
      const ScheduleUploadBlobOptions& options)
  {
    (void)options;

    // TODO: remove SAS credential from url
    auto pair = CreateJob(
        TransferType::SingleUpload,
        _internal::GetFileUrl(sourceLocalPath),
        destinationBlob.GetUrl());
    auto& jobProperties = pair.first;
    auto& rootTask = pair.second;

    auto task = rootTask->CreateTask<_detail::UploadBlobFromFileTask>(
        _internal::TaskType::NetworkUpload, sourceLocalPath, destinationBlob);

    m_scheduler.AddTask(std::move(task));

    return std::move(jobProperties);
  }

  JobProperties BlobTransferManager::ScheduleUploadDirectory(
      const std::string& sourceLocalPath,
      const BlobFolder& destinationBlobFolder,
      const ScheduleUploadBlobOptions& options)
  {
    (void)options;

    auto pair = CreateJob(
        TransferType::DirectoryUpload,
        _internal::GetFileUrl(sourceLocalPath),
        destinationBlobFolder.GetUrl());
    auto& jobProperties = pair.first;
    auto& rootTask = pair.second;

    auto task = rootTask->CreateTask<_detail::UploadBlobsFromDirectoryTask>(
        _internal::TaskType::NetworkUpload, sourceLocalPath, destinationBlobFolder);

    m_scheduler.AddTask(std::move(task));

    return std::move(jobProperties);
  }

  JobProperties BlobTransferManager::ScheduleDownload(
      const BlobClient& sourceBlob,
      const std::string& destinationLocalPath,
      const ScheduleDownloadBlobOptions& options)
  {
    (void)options;

    auto pair = CreateJob(
        TransferType::SingleDownload,
        sourceBlob.GetUrl(),
        _internal::GetFileUrl(destinationLocalPath));
    auto& jobProperties = pair.first;
    auto& rootTask = pair.second;

    auto task = rootTask->CreateTask<_detail::DownloadBlobToFileTask>(
        _internal::TaskType::NetworkDownload, sourceBlob, destinationLocalPath);

    m_scheduler.AddTask(std::move(task));

    return std::move(jobProperties);
  }

}}} // namespace Azure::Storage::Blobs
