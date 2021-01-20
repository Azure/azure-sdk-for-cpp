// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  struct DownloadBlobToResult
  {
    std::string ETag;
    Azure::Core::DateTime LastModified;
    int64_t ContentLength = 0;
    BlobHttpHeaders HttpHeaders;
    Storage::Metadata Metadata;
    Models::BlobType BlobType;
    bool IsServerEncrypted = false;
    Azure::Core::Nullable<std::vector<uint8_t>> EncryptionKeySha256;
  };

  using UploadBlockBlobFromResult = UploadBlockBlobResult;

}}}} // namespace Azure::Storage::Blobs::Models
