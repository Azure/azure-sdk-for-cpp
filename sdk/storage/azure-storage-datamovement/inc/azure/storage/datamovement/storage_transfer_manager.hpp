// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/storage/blobs.hpp>

#include "azure/storage/datamovement/blob_folder.hpp"
#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/job_properties.hpp"

namespace Azure { namespace Storage { namespace DataMovement {

  class StorageTransferManager final {
  public:
    StorageTransferManager(const StorageTransferManagerOptions& options);

    JobProperties ScheduleUpload(
        const std::string& sourceLocalPath,
        const Blobs::BlobClient& destinationBlob,
        const UploadBlobOptions& options = UploadBlobOptions());

    JobProperties ScheduleUploadDirectory(
        const std::string& sourceLocalPath,
        const BlobFolder& destinationBlobFolder,
        const UploadBlobOptions& options = UploadBlobOptions());

    JobProperties ScheduleDownload(
        const Blobs::BlobClient& sourceBlob,
        const std::string& destinationLocalPath,
        const DownloadBlobOptions& options = DownloadBlobOptions());

    JobProperties ScheduleDownloadDirectory(
        const BlobFolder& sourceBlobFolder,
        const std::string& destinationLocalPath,
        const DownloadBlobOptions& options = DownloadBlobOptions());

    JobProperties GetJobProperties(const std::string& jobId);
    void PauseJob(const std::string& jobId);
    void PauseAllJobs();
    void ResumeJob(const std::string& jobId);
    void ResumeAllJobs();
    void CancelJob(const std::string& jobId);
    void CancelAllJobs();

  private:
  };

}}} // namespace Azure::Storage::DataMovement
