// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/storage/datamovement/blob_folder.hpp"
#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/storage_transfer_manager.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  class BlobTransferManager final : public StorageTransferManager {
  public:
    JobProperties ScheduleUpload(
        const std::string& sourceLocalPath,
        const BlobClient& destinationBlob,
        const ScheduleUploadBlobOptions& options = ScheduleUploadBlobOptions());

    JobProperties ScheduleUploadDirectory(
        const std::string& sourceLocalPath,
        const BlobFolder& destinationBlobFolder,
        const ScheduleUploadBlobOptions& options = ScheduleUploadBlobOptions());

    JobProperties ScheduleDownload(
        const BlobClient& sourceBlob,
        const std::string& destinationLocalPath,
        const ScheduleDownloadBlobOptions& options = ScheduleDownloadBlobOptions());

    JobProperties ScheduleDownloadDirectory(
        const BlobFolder& sourceBlobFolder,
        const std::string& destinationLocalPath,
        const ScheduleDownloadBlobOptions& options = ScheduleDownloadBlobOptions());
  };

}}} // namespace Azure::Storage::Blobs
