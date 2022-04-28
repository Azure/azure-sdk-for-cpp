// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace DataMovement {

  enum class TransferType
  {
    SingleUpload = 0,
    SingleDownload = 1,
    DirectoryUpload = 2,
    DirectoryDownload = 3,
  };

  struct JobProperties final
  {
    std::string JobId;
    std::string SourceUrl;
    std::string DestinationUrl;
    TransferType Type;
  };

}}} // namespace Azure::Storage::DataMovement
