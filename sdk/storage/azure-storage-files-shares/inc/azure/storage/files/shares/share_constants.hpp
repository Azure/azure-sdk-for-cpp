
//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  constexpr static const char* FileDefaultTimeValue = "now";
  constexpr static const char* FileCopySourceTime = "source";
  constexpr static const char* FileInheritPermission = "inherit";
  constexpr static const char* FilePreserveSmbProperties = "preserve";
  constexpr static const char* FileAllHandles = "*";

  namespace _detail {
    constexpr static const char* ShareSnapshotQueryParameter = "sharesnapshot";

    // Error codes:
    constexpr static const char* ParentNotFound = "ParentNotFound";
    constexpr static const char* ResourceNotFound = "ResourceNotFound";
    constexpr static const char* ShareAlreadyExists = "ShareAlreadyExists";
    constexpr static const char* ShareNotFound = "ShareNotFound";
    constexpr static const char* ResourceAlreadyExists = "ResourceAlreadyExists";
  } // namespace _detail

}}}} // namespace Azure::Storage::Files::Shares
