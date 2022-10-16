// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <memory>
#include <string>

#include <azure/core/credentials/credentials.hpp>
#include <azure/storage/common/storage_credential.hpp>

namespace Azure { namespace Storage {
  struct StorageTransferManagerOptions final
  {
    std::string TransferStateDirectoryPath = ".";
    Nullable<int> NumThreads;
    Nullable<size_t> MaxMemorySize;
  };

  struct TransferCredential final
  {
    std::string SasCredential; // TODO: or Azure SAS credential?
    std::shared_ptr<StorageSharedKeyCredential> SharedKeyCredential;
    std::shared_ptr<Core::Credentials::TokenCredential> TokenCredential;
  };

  struct TransferProgress final
  {
    int64_t NumFilesTransferred = 0;
    int64_t NumFilesSkipped = 0;
    int64_t NumFilesFailed = 0;
    int64_t TotalBytesTransferred = 0;
  };

  struct TransferError final
  {
    std::string SourceUrl;
    std::string DestinationUrl;
    // TODO: error information: std::exception_ptr or descriptive string?
  };

  struct ResumeJobOptions final
  {
    TransferCredential SourceCredential;
    TransferCredential DestinationCredential;
    std::function<void(const TransferProgress&)> ProgressHandler;
    std::function<void(TransferError&)> ErrorHandler;
  };

  namespace Blobs {
    struct ScheduleUploadBlobOptions final
    {
      std::function<void(const TransferProgress&)> ProgressHandler;
      std::function<void(TransferError&)> ErrorHandler;
    };

    struct ScheduleDownloadBlobOptions final
    {
      std::function<void(const TransferProgress&)> ProgressHandler;
      std::function<void(TransferError&)> ErrorHandler;
    };

    struct ScheduleCopyBlobOptions final
    {
      std::function<void(const TransferProgress&)> ProgressHandler;
      std::function<void(TransferError&)> ErrorHandler;
    };
  } // namespace Blobs
}} // namespace Azure::Storage
