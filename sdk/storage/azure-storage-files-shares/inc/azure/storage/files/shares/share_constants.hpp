
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  constexpr static const char* c_FileDefaultTimeValue = "now";
  constexpr static const char* c_FileCopySourceTime = "source";
  constexpr static const char* c_FileInheritPermission = "inherit";
  constexpr static const char* c_FilePreserveSmbProperties = "preserve";
  constexpr static const char* c_FileAllHandles = "*";

  namespace Details {
    constexpr int64_t c_FileUploadDefaultChunkSize = 4 * 1024 * 1024;
    constexpr int64_t c_FileDownloadDefaultChunkSize = 4 * 1024 * 1024;
    constexpr static const char* c_ShareSnapshotQueryParameter = "sharesnapshot";

    // Error codes:
    constexpr static const char* ParentNotFound = "ParentNotFound";
    constexpr static const char* ResourceNotFound = "ResourceNotFound";
    constexpr static const char* ShareAlreadyExists = "ShareAlreadyExists";
    constexpr static const char* ShareNotFound = "ShareNotFound";
    constexpr static const char* ResourceAlreadyExists = "ResourceAlreadyExists";
  } // namespace Details

}}}} // namespace Azure::Storage::Files::Shares
