
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  constexpr static const char* c_FileDefaultTimeValue = "now";
  constexpr static const char* c_FileInheritPermission = "inherit";
  constexpr static const char* c_FilePreserveSmbProperties = "preserve";

  namespace Details {
    constexpr int64_t c_FileUploadDefaultChunkSize = 4 * 1024 * 1024;
    constexpr int64_t c_FileDownloadDefaultChunkSize = 4 * 1024 * 1024;
    constexpr static const char* c_ShareSnapshotQueryParameter = "sharesnapshot";
  } // namespace Details

}}}} // namespace Azure::Storage::Files::Shares
