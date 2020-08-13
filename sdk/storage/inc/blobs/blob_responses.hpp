// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blobs/copy_blob_poller.hpp"
#include "protocol/blob_rest_client.hpp"

#include <map>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs {

  using UserDelegationKey = GetUserDelegationKeyResult;

  struct DownloadBlobToResult
  {
    std::string ETag;
    std::string LastModified;
    int64_t ContentLength = 0;
    BlobHttpHeaders HttpHeaders;
    std::map<std::string, std::string> Metadata;
    Blobs::BlobType BlobType = Blobs::BlobType::Unknown;
    Azure::Core::Nullable<bool> ServerEncrypted;
    Azure::Core::Nullable<std::string> EncryptionKeySha256;
  };

  using UploadBlockBlobFromBufferResult = UploadBlockBlobResult;
  using UploadBlockBlobFromFileResult = UploadBlockBlobResult;

  struct PageRange
  {
    int64_t Offset;
    int64_t Length;
  };

  struct GetPageBlobPageRangesResult
  {
    std::string ETag;
    std::string LastModified;
    int64_t BlobContentLength = 0;
    std::vector<PageRange> PageRanges;
    std::vector<PageRange> ClearRanges;
  };

  struct StartCopyBlobFromUriResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::CopyStatus CopyStatus = Blobs::CopyStatus::Unknown;
    Azure::Core::Nullable<std::string> VersionId;
    CopyBlobPoller Poller;
  };

  struct StartCopyPageBlobIncrementalResult
  {
    std::string ETag;
    std::string LastModified;
    std::string CopyId;
    Blobs::CopyStatus CopyStatus = Blobs::CopyStatus::Unknown;
    Azure::Core::Nullable<std::string> VersionId;
    CopyBlobPoller Poller;
  };

}}} // namespace Azure::Storage::Blobs
