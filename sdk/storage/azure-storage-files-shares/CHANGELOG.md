# Release History

## 12.0.0-beta.9 (Unreleased)

### New Features

- Added support for customized application ID.

## 12.0.0-beta.8 (2021-02-12)

### New Features

- Changed type of `FileAttributes` to extensible enum.

### Breaking Changes

- `ListSharesSinglePageOptions::ListSharesInclude` was renamed to `ListSharesSinglePageOptions::ListSharesIncludeFlags`.
- `DeleteShareOptions::IncludeSnapshots` was renamed to `DeleteShareOptions::DeleteSnapshots`.
- `FileShareSmbProperties` was renamed to `FileSmbProperties`.
- `DownloadShareFileOptions::GetRangeContentMd5` was renamed to `DownloadShareFileOptions::RangeHashAlgorithm`.
- `UploadFileRangeFromUriOptions::SourceContentHash` was renamed to `UploadFileRangeFromUriOptions::TransactionalContentHash`.
- `GetShareFileRangeListOptions::PrevShareSnapshot` was renamed to `GetShareFileRangeListOptions::PreviousShareSnapshot`.
- Refined `CreateShareDirectoryResult` and `CreateShareFileResult`.
- Removed `DownloadShareFileDetails::AcceptRanges`.
- Removed `GetShareFilePropertiesResult::FileType`.
- Added `RequestId` in `ForceCloseShareDirectoryHandleResult`.
- Removed `TransactionalContentHash` from `ClearShareFileRangeResult`.
- Changed API signature of `ShareFileClient::UploadRangeFromUri`.
- Renamed `ForceCloseAllHandles` to `ForceCloseAllHandlesSinglePage` and all related structs.
- Made all `ContinuationToken` in return types nullable.
- Renamed `ShareFileHttpHeaders` to `FileHttpHeaders`.
- Renamed `ShareGetPropertiesResult::AccessTierChangeTime` to `AccessTierChangedOn`.
- Renamed `ShareGetStatisticsResult::ShareUsageBytes` to `ShareUsageInBytes`.
- Renamed `ShareGetPermissionResult::Permission` to `FilePermission`.
- Grouped all file SMB properties into a struct and refined the APIs that return these properties.
- Renamed `numberOfHandlesClosed` to `NumberOfHandlesClosed` and `numberOfHandlesFailedToClose` to `NumberOfHandlesFailedToClose`.
- Renamed `FileGetRangeListResult::FileContentLength` to `FileSize`.
- Renamed `StorageServiceProperties` to `FileServiceProperties`.
- Removed `LeaseTime` in results returned by lease operations. Also removed `LeaseId` in `ShareBreakLeaseResult`.
- Moved `Azure::Core::Context` out of options bag of each API, and make it the last optional parameter.

## 12.0.0-beta.7 (2021-02-04)

### New Features

- Added support for `UploadRangeFromUri` in file client.
- Added support for `SetProperties` in share client. This API supports update share tier and adjusting share's quota.
- Added support to get share's tier status in `ListSharesSinglePage` and `GetProperties`.
- Added `ChangedOn`, `FileId`, `ParentId` to the `FileShareSmbProperties`.

### Breaking Changes

- Removed `GetDirectoryClient` and `GetFileClient` from `ShareClient`. `ShareDirectoryClient` and `ShareFileClient` now initializes with the name of the resource, not path, to indicate that no path parsing is done for the API
- `ContentRange` in `FileDownloadResult` is now `Azure::Core::Http::Range`.
- `ContentLength` in `FileDownloadResult` is renamed to `FileSize`.
- Renamed `GetUri` to `GetUrl`.
- Moved all protocol layer generated result types to `Details` namespace.
- Renamed `ShareItems` in `ListSharesResponse` to `Items`.
- Renamed `ShareItems` in `ServiceListSharesSinglePageResult` to `Items`.
- Added `ShareLeaseClient`, all lease related APIs are moved to `ShareLeaseClient`.
- Changed lease duration to be `std::chrono::seconds`.
- Added `RequestId` in each return types for REST API calls, except for concurrent APIs.
- Removed `PreviousContinuationToken` from `ListFilesAndDirectoriesSinglePageResult` and `ListSharesSinglePageResult`.
- Removed `c_` for constants: `c_FileDefaultTimeValue`, `c_FileCopySourceTime`, `c_FileInheritPermission`, `FilePreserveSmbProperties` and `FileAllHandles`.
- `Concurrency`, `ChunkSize` and `InitialChunkSize` were moved into `DownloadShareFileToOptions::TansferOptions`.
- `Concurrency`, `ChunkSize` and `SingleUploadThreshold` were moved into `UploadShareFileFromOptions::TransferOptions`.
- Removed `SetQuota` related API, result and options. The functionality is moved into `SetProperties`.
- Moved some less commonly used properties returned when downloading a file into a new structure called `DownloadShareFileDetails`. This will impact the return type of `ShareFileClient::Download` and `ShareFileClient::DownloadTo`.
- Renamed `FileProperty` to `FileItemDetails` to align with other SDK's naming pattern for returned items for list operation.
- Renamed `ShareProperties` to `ShareItemDetails` to align with other SDK's naming pattern for returned items for list operation.

### Other Changes and Improvements

- Removed `c_` for constants and renamed to pascal format.

## 12.0.0-beta.6 (2020-01-14)

### New Features

- Added support for `CreateIfNotExists` for Share and Directory clients, and `DeleteIfExists` for Share, Directory and File clients.
- Support setting file SAS permission with a raw string.

### Breaking Changes

- Removed constructors in clients that takes a `Azure::Identity::ClientSecretCredential`.
- Removed Share Lease related APIs such as `ShareClient::AcquireLease` and `ReleaseLease` since they are not supported in recent service versions.
- Moved File SAS into `Azure::Storage::Sas` namespace.
- Replaced all transactional content MD5/CRC64 with the `ContentHash` struct.
- `FileShareHttpHeaders` is renamed to `ShareFileHttpHeaders`, and member `std::string ContentMd5` is changed to `Storage::ContentHash ContentHash`.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- Moved version strings into `Details` namespace.
- Renamed all functions and structures that could retrieve partial query results from the server to have `SinglePage` suffix instead of `Segment` suffix.
- Removed `FileRange`, `ClearRange`, and `Offset` and `Length` pair in options. They are now represented with `Azure::Core::Http::Range`.
- Replace scoped enums that don't support bitwise operations with extensible enum.
- `IsServerEncrypted` members in `DownloadFileToResult`, `UploadFileFromResult`, `FileDownloadResult` and `FileGetPropertiesResult` are no longer nullable.
- Create APIs for Directory and File now returns `FileShareSmbProperties` that aggregates SMB related properties.
- `DirectoryClient` is renamed to `ShareDirectoryClient`, `FileClient` is renamed to `ShareFileClient`.
- Directory and File related result types and options types now have a `Share` prefix. For example, `SetDirectoryPropertiesResult` is changed to `SetShareDirectoryPropertiesResult`.
- Renamed `GetSubDirectoryClient` to `GetSubdirectoryClient`.
- Type for ETag was changed to `Azure::Core::ETag`.

## 12.0.0-beta.5 (2020-11-13)

### Breaking Changes

- `Azure::Storage::Files::Shares::Metrics::IncludeAPIs` is now renamed to `Azure::Storage::Files::Shares::Metrics::IncludeApis`, and is changed to a nullable member.
- Moved header `azure/storage/files/shares/shares.hpp` to `azure/storage/files/shares.hpp`.
- Moved returning model types and related functions in `Azure::Storage::Files::Shares` to `Azure::Storage::Files::Shares::Models`, and made other code private by moving it into `Azure::Storage::Files::Shares::Details`.
- Renamed `Azure::Storage::Files::Shares::ServiceClient` to `Azure::Storage::Files::Shares::ShareServiceClient`.

## 1.0.0-beta.4 (2020-10-16)

### New Features

- Service version is now 2020-02-10.
- Added support for leasing a share:
  - ShareClient::AcquireLease
  - ShareClient::ReleaseLease
  - ShareClient::ChangeLease
  - ShareClient::BreakLease
  - ShareClient::RenewLease

### Breaking Changes

- `CreateFromConnectionString` now accepts unencoded file and directory name.
- Added support for getting range list with previous snapshot. `GetFileRangeListResult` now returns `std::vector<FileRange> Ranges` and `std::vector<FileRange> ClearRanges` instead of `std::vector<Range> RangeList`.
- Added support for SMB Multi-Channel setting for `ServiceClient::GetProperties` and `ServiceClient::SetProperties`. This is only available for Storage account with Premium File access.
  - Standard account user has to remove the returned SMB Multi-Channel setting before set, otherwise service would return failure.
- `Marker` is renamed to `ContinuationToken` in options.
- `NextMarker` is renamed to `ContinuationToken` in returned result objects.
- `Marker` is renamed to `PreviousContinuationToken` in returned result objects.

### Bug Fixes

- Unencoded Share/File/Directory name is now encoded.

## 1.0.0-beta.2 (2020-09-09)

### New Features

- Added File SAS generation support.
- Release based on azure-core_1.0.0-beta.1.

## 1.0.0-beta.1 (2020-08-28)

### New Features

- Added support for File features:
  - ServiceClient::ListSharesSegment
  - ServiceClient::SetProperties
  - ServiceClient::GetProperties
  - ShareClient::Create
  - ShareClient::Delete
  - ShareClient::CreateSnapshot
  - ShareClient::GetProperties
  - ShareClient::SetQuota
  - ShareClient::SetMetadata
  - ShareClient::GetAccessPolicy
  - ShareClient::SetAccessPolicy
  - ShareClient::GetStatistics
  - ShareClient::CreatePermission
  - ShareClient::GetPermission
  - ShareClient::ListFilesAndDirectoriesSegment
  - DirectoryClient::Create
  - DirectoryClient::Delete
  - DirectoryClient::GetProperties
  - DirectoryClient::SetProperties
  - DirectoryClient::SetMetadata
  - DirectoryClient::ListFilesAndDirectoriesSegment
  - DirectoryClient::ListHandlesSegment
  - DirectoryClient::ForceCloseHandle
  - DirectoryClient::ForceCloseAllHandles
  - FileClient::Create
  - FileClient::Delete
  - FileClient::Download
  - FileClient::DownloadTo
  - FileClient::UploadFrom
  - FileClient::StartCopy
  - FileClient::AbortCopy
  - FileClient::GetProperties
  - FileClient::SetProperties
  - FileClient::SetMetadata
  - FileClient::UploadRange
  - FileClient::ClearRange
  - FileClient::GetRangeList
  - FileClient::ListHandlesSegment
  - FileClient::ForceCloseHandle
  - FileClient::ForceCloseAllHandles
  - FileClient::AcquireLease
  - FileClient::ReleaseLease
  - FileClient::ChangeLease
  - FileClient::BreakLease
