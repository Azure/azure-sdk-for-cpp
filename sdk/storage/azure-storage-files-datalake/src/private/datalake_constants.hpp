// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace _detail {

  constexpr static const char* ContainerAlreadyExists = "ContainerAlreadyExists";
  constexpr static const char* ContainerNotFound = "ContainerNotFound";
  constexpr static const char* DataLakeFilesystemNotFound = "FilesystemNotFound";
  constexpr static const char* DataLakePathNotFound = "PathNotFound";
  constexpr static const char* DataLakePathAlreadyExists = "PathAlreadyExists";
  constexpr static const char* DataLakeIsDirectoryKey = "hdi_isFolder";
  constexpr static const char* EncryptionContextHeaderName = "x-ms-encryption-context";
  constexpr static const char* OwnerHeaderName = "x-ms-owner";
  constexpr static const char* GroupHeaderName = "x-ms-group";
  constexpr static const char* PermissionsHeaderName = "x-ms-permissions";

}}}}} // namespace Azure::Storage::Files::DataLake::_detail
