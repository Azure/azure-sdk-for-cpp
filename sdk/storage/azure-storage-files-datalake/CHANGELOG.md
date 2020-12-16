# Release History

## 12.0.0-beta.6 (Unreleased)

### Breaking Changes

- Move DataLake SAS into `Azure::Storage::Sas` namespace.

## 12.0.0-beta.5 (2020-11-13)

### Breaking Changes

- Moved header `azure/storage/files/datalake/datalake.hpp` to `azure/storage/files/datalake.hpp`.
- Moved returning model types and related functions in `Azure::Storage::Files::DataLake` to `Azure::Storage::Files::DataLake::Models`, and made other code private by moving it into `Azure::Storage::Files::DataLake::Details`.
- Renamed `Azure::Storage::Files::DataLake::ServiceClient` to `Azure::Storage::Files::DataLake::DataLakeServiceClient`.

## 1.0.0-beta.4 (2020-10-16)

### New Features

- Service version is now 2020-02-10.
- Added support for SAS generation in DataLake service.
- Added support for `FileClient::ScheduleDeletion`.
- Added support for `DirectoryClient::SetAccessControlRecursive`.
- `PathAppendDataResult` now returns `ContentMD5`, `ContentCrc64` and `IsServerEncrypted`.

### Breaking Changes

- `CreateFromConnectionString` now accepts unencoded file, path and directory name.
- `ETag` and `LastModified` is now `std::string` instead of `Azure::Core::Nullable<std::string>` in `CreateDirectoryResult`, `CreateFileResult` and `CreatePathResult`.
- `Continuation` is renamed to `ContinuationToken` in options and returned result objects.

### Bug Fixes

- Unencoded FileSystem/File/Path/Directory name is now encoded.

## 1.0.0-beta.2 (2020-09-09)

### New Features

- Release based on azure-core_1.0.0-beta.1.

## 1.0.0-beta.1

### New Features

- Support for Lease related operations.

## 1.0.0-preview.1 (Unreleased)

### New Features

- Added support for DataLake features:
  - ServiceClient::ListFileSystems
  - ServiceClient::GetUserDelegationKey
  - FileSystemClient::Create
  - FileSystemClient::Delete
  - FileSystemClient::SetMetadata
  - FileSystemClient::GetProperties
  - FileSystemClient::ListPaths
  - PathClient::Create
  - PathClient::Delete
  - PathClient::SetAccessControl
  - PathClient::SetHttpHeaders
  - PathClient::GetProperties
  - PathClient::GetAccessControls
  - PathClient::SetMetadata
  - FileClient::AppendData
  - FileClient::FlushData
  - FileClient::Create
  - FileClient::Rename
  - FileClient::Delete
  - FileClient::Read
  - FileClient::UploadFromBuffer
  - FileClient::UploadFromFile
  - FileClient::DownloadToBuffer
  - FileClient::DownloadToFile
  - DirectoryClient::GetFileClient
  - DirectoryClient::Create
  - DirectoryClient::Rename
  - DirectoryClient::Delete
