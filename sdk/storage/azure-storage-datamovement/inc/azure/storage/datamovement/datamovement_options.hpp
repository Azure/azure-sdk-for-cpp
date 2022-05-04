// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Storage {
  struct StorageTransferManagerOptions final
  {
    std::string TransferStateDirectoryPath;
  };

  namespace Blobs {

    struct ScheduleUploadBlobOptions final
    {
    };

    struct ScheduleDownloadBlobOptions final
    {
    };
  } // namespace Blobs
}} // namespace Azure::Storage
