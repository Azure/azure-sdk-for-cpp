
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Details {
  // Error codes:
  constexpr static const char* ContainerAlreadyExists = "ContainerAlreadyExists";
  constexpr static const char* ContainerNotFound = "ContainerNotFound";
  constexpr static const char* DataLakeFilesystemNotFound = "FilesystemNotFound";
  constexpr static const char* DataLakePathNotFound = "PathNotFound";
  constexpr static const char* DataLakePathAlreadyExists = "PathAlreadyExists";

}}}}} // namespace Azure::Storage::Files::DataLake::Details
