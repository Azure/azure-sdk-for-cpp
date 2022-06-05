// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/blob_folder.hpp"

#include <vector>

#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobFolder::BlobFolder(BlobContainerClient blobContainerClient, std::string folderPath)
      : m_blobContainerClient(std::move(blobContainerClient)), m_folderPath(std::move(folderPath))
  {
    if (m_folderPath == "/")
    {
      m_folderPath.clear();
    }

    if (!m_folderPath.empty()
      && m_folderPath[m_folderPath.length() - 1] != '/')
    {
      m_folderPath += '/';
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
