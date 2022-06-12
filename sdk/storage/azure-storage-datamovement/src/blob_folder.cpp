// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/blob_folder.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobFolder::BlobFolder(BlobContainerClient blobContainerClient, std::string folderPath)
      : m_blobContainerClient(std::move(blobContainerClient)), m_folderPath(std::move(folderPath))
  {
    if (m_folderPath == "/")
    {
      m_folderPath.clear();
    }
  }

  std::string BlobFolder::GetUrl() const
  {
    auto url = Azure::Core::Url(m_blobContainerClient.GetUrl());
    url.AppendPath(m_folderPath);
    return url.GetAbsoluteUrl();
  }

  BlobFolder BlobFolder::GetBlobFolder(const std::string& folderName) const
  {
    return BlobFolder(
        m_blobContainerClient, m_folderPath + (m_folderPath.empty() ? "" : "/") + folderName);
  }

  BlobClient BlobFolder::GetBlobClient(const std::string& blobName) const
  {
    return m_blobContainerClient.GetBlobClient(
        m_folderPath + (m_folderPath.empty() ? "" : "/") + blobName);
  }
}}} // namespace Azure::Storage::Blobs
