// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Storage { namespace DataMovement {

  struct StorageTransferManagerOptions final
  {
    std::string TransferStateDirectoryPath;
  };

  struct UploadBlobOptions final
  {
  };

  struct DownloadBlobOptions final
  {
  };
}}} // namespace Azure::Storage::DataMovement
