// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Blobs {
  namespace _detail {
    struct DownloadBlobDirectoryTask;
  }

  class BlobFolder final {
  public:
    explicit BlobFolder(BlobContainerClient blobContainerClient, std::string folderPath);

    std::string GetUrl() const;

    BlobFolder GetBlobFolder(const std::string& folderName) const;

    BlobClient GetBlobClient(const std::string& blobName) const;

  private:
    BlobContainerClient m_blobContainerClient;
    std::string m_folderPath;

    friend struct _detail::DownloadBlobDirectoryTask;
  };

}}} // namespace Azure::Storage::Blobs
