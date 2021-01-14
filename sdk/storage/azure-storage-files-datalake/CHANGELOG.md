# Release History

## 12.0.0-beta.6 (2020-01-14)

### New Features

- Support setting DataLake SAS permission with a raw string.
- Added support for `CreateIfNotExists` and `DeleteIfExists` for FileSystem, Path, Directory and File clients.

### Breaking Changes

- Moved DataLake SAS into `Azure::Storage::Sas` namespace.
- `EncrytionKeySha256` are changed to binary (`std::vector<uint8_t>`).
- Replaced all transactional content MD5/CRC64 with the `ContentHash` struct.
- `DataLakeHttpHeaders` is renamed to `PathHttpHeaders`, and now contains `ContentHash` for the resource.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- `CreationTime` is renamed to `CreatedOn`.
- `AccessTierChangeTime` is renamed to `AccessTierChangedOn`.
- `CopyCompletionTime` is renamed to `CopyCompletedOn`.
- `ExpiryTime` is renamed to `ExpiresOn`.
- `LastAccessTime` is renamed to `LastAccessedOn`.
- Made version strings private by moving them into the `Details` namespace.
- Renamed all functions and structures that could retrieve partial query results from the server to have `SinglePage` suffix instead of `Segment` suffix.
- `ReadFileResult` now have `ContentRange` as string.
- `ReadFileOptions` now have `Azure::Core::Http::Range` instead of `Content-Length` and `Offset`.
- Replaced scoped enums that don't support bitwise operations with extensible enum.
- `ListPaths` is renamed to `ListPathsSinglePage` and its related return type and options are also renamed accordingly.
- Added `DataLake` prefix to `FileSystemClient`, `PathClient`,  `DirectoryClient`, and `FileClient` types.
- FileSystems, Path, Directory and File related result types and options types now have a `DataLake` prefix. For example, `GetFileSystemPropertiesResult` is changed to `GetDataLakeFileSystemPropertiesResult`.
- Renamed `GetSubDirectoryClient` to `GetSubdirectoryClient`.
- Removed `NamespaceEnabled` field in `CreateDataLakeFileSystemResult`.

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

## 1.0.0-beta.1 (2020-08-28)

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
- Support for Lease related operations.
