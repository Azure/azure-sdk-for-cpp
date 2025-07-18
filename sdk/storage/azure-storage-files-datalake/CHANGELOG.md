# Release History

## 12.13.0-beta.2 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

### Other Changes

## 12.13.0-beta.1 (2025-06-24)

### Features Added

- Added more useful error message when the SDK encounters an x-ms-version mis-match issue.

## 12.12.0 (2024-09-17)

### Features Added

- Features in `12.12.0-beta.1` are now generally available.

## 12.12.0-beta.1 (2024-08-07)

### Features Added

- Added ability to retrieve SAS string to sign for debugging purposes.

## 12.11.0 (2024-07-16)

### Features Added

- Features in `12.11.0-beta.1` are now generally available.

## 12.11.0-beta.1 (2024-06-11)

### Features Added

- Bumped up API version to `2024-08-04`.
- Added more detailed messaging for authorization failure cases.

## 12.10.0 (2024-05-07)

### Features Added

- Features in `12.10.0-beta.1` are now generally available.

### Bugs Fixed

- Fixed a bug where `PathItem::EncryptionContext` returned by `DataLakeDirectoryClient::ListPaths` was always null.

## 12.10.0-beta.1 (2024-04-17)

### Features Added

- Bumped up API version to `2023-11-03`.
- Added new field `IncludeUserPrincipalName` in `GetPathAccessControlListOptions`, `GetPathPropertiesOptions` and `DownloadFileOptions`.
- Added new field `Acls` in `PathProperties` and `DownloadFileDetails`.

## 12.9.0 (2023-11-07)

### Features Added

- Features in `12.9.0-beta.1` are now generally available.

## 12.9.0-beta.1 (2023-10-17)

### Features Added

- Added new extendable enum `DataLakeAudience`
- Added new field `Audience` in `DataLakeClientOptions`

## 12.8.0 (2023-09-12)

### Features Added

- Features in `12.8.0-beta.1` and `12.8.0-beta.2` are now generally available.

## 12.8.0-beta.2 (2023-08-12)

### Features Added

- TenantId can now be discovered through the service challenge response, when using a TokenCredential for authorization.
    - A new property is now available on `DataLakeClientOptions` called `EnableTenantDiscovery`. If set to `true`, the client will attempt an initial unauthorized request to the service to prompt a challenge containing the tenantId hint.

## 12.8.0-beta.1 (2023-08-08)

### Features Added

- Bumped up API version to `2023-08-03`.
- Added support for paginated directory delete when using AAD authentication.

### Bugs Fixed

- Fixed a bug where `DataLakeDirectoryClient::ListPaths` and `DataLakeFileSystemClient::ListPaths` cannot list the second page and always fail with `std::bad_function_call`.
- Fixed a bug where `DataLakePathClient::SetAccessControlListRecursive` crashes when operating on the second page.

## 12.7.0 (2023-07-11)

### Features Added

- New features in `12.7.0-beta.1` are now generally available.

## 12.7.0-beta.1 (2023-05-31)

### Features Added

- Added new fields `Owner`, `Group` and `Permissions` in `PathProperties` and `DownloadFileDetails`.

## 12.6.0 (2023-05-09)

### Features Added

- New features in `12.6.0-beta.1` are now generally available.

## 12.6.0-beta.1 (2023-04-11)

### Features Added
- Added support for encryption context:
  - new field `EncryptionContext` in `CreatePathOptions` , `PathItem`, `PathProperties`,  `DownloadFileDetails`.

## 12.5.0 (2023-01-10)

### Features Added

- New features in `12.5.0-beta.1` are now generally available.


## 12.5.0-beta.1 (2022-11-08)

### Features Added
- Added support for encryption scope:
  - Added new fields `DefaultEncryptionScope` and `PreventEncryptionScopeOverride` in `FileSystemItemDetails`, `FileSystemProperties` and `CreateFileSystemOptions`.
  - Added new field `EncryptionScope` in `PathProperties`.

## 12.4.0 (2022-11-08)

### Features Added

- New features in `12.4.0-beta.1` are now generally available.

## 12.4.0-beta.1 (2022-10-11)

### Features Added

- Added support for encryption scope in SAS builder.
- Added support for customer-provided key.
- Added support for flushing when appending data to a file.
- Added some new fields in `CreatePathOptions`.
- New APIs:
  - DataLakeServiceClient::SetProperties
  - DataLakeServiceClient::GetProperties
  - DataLakeFileSystemClient::ListDeletedPaths
  - DataLakeFileSystemClient::UndeletePath
  - DataLakeFileClient::Query

### Bugs Fixed

- Fixed a bug where file/directory renaming cannot be authenticated with SAS.
- Empty file or existing file won't be created/overwritten if the file to be downloaded doesn't exist.

## 12.3.1 (2022-03-09)

### Other Changes

- No public changes in this release.

## 12.3.0 (2022-02-14)

### Other Changes

- Deprecated enum `LeaseDuration`, use `LeaseDurationType` instead.

## 12.2.0 (2021-09-08)

### Breaking Changes

- `StartsOn` and `ExpiresOn` in `SignedIdentifier` were changed to nullable.

### Bugs Fixed

- Fixed a bug where prefix cannot contain `&` when listing files.

### Other Changes

- Create less threads if there isn't too much data to transfer.

## 12.1.0 (2021-08-10)

### Features Added

- Added `ETag` and `LastModified` into `ScheduleFileDeletionResult`.

## 12.0.1 (2021-07-07)

### Bug Fixes

- Fixed a bug where transactional MD5 hash was treated as blob MD5 hash when downloading partial blob.

## 12.0.0 (2021-06-08)

### Breaking Changes

- Renamed `ContentLength` in `FlushFileResult` to `FileSize`.

### Other Changes and Improvements

- Removed `IfUnmodifiedSince` from access conditions of setting filesystem metadata operation.
- Updated some samples.
- Fixed a read consistency issue.

## 12.0.0-beta.11 (2021-05-19)

### New Features

- Added `DataLakePathClient::SetAccessControlListRecursive()`, `UpdateAccessControlListRecursive()` and `RemoveAccessControlListRecursive()`.

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Renamed `HasMorePages()` in paged response to `HasPage()`.
- Default chunk size for concurrent upload was changed to nullable.
- `DataLakeLeaseClient::Change()` updates internal lease id.

## 12.0.0-beta.10 (2021-04-16)

### Breaking Changes

- Removed `Azure::Storage::Files::DataLake::PackageVersion`.
- Renamed `GetUserDelegationKeyOptions::startsOn` to `StartsOn`.
- Replaced all paginated collection functions that have the SinglePage suffix with pageable functions returning a `PagedResponse<T>`-derived type. The options are also renamed accordingly.
  - `DataLakeServiceClient::ListFileSystems()`.
  - `DataLakeFileSystemClient::ListPaths()`.
  - `DataLakeDirectoryClient::ListPaths()`.
- Removed `DataLakePathClient::SetAccessControlListRecursiveSinglePage()`, `UpdateAccessControlListRecursiveSinglePage()` and `RemoveAccessControlListRecursiveSinglePage()`.

### Bug Fixes

- Rename functions always fail because `/` was left out in the renamed source path.

## 12.0.0-beta.9 (2021-03-23)

### New Features

- Added support for telemetry options.
- Added `Azure::Storage::Files::DataLake::PackageVersion`.

### Breaking Changes

- DataLake client constructors won't automatically convert blob url to dfs url anymore.
- String conversion functions of extensible enums were renamed from `Get()` to `ToString()`.
- Moved `SecondaryHostForRetryReads` out of retry options, now it's under `DataLakeClientOptions`.
- Changed the return types of the following APIs:
  - `DataLakeServiceClient::GetUserDelegationKey` now returns `UserDelegationKey`.
  - `DataLakeFileSystemClient::GetProperties` now returns  `FileSystemProperties`.
  - `DataLakeFileSystemClient::GetAccessPolicy` now returns  `FileSystemAccessPolicy`.
  - `DataLakePathClient::GetProperties` now returns `PathProperties`.
  - `DataLakePathClient::GetAccessControlList` now returns `PathAccessControlList`.
- Removed `GetUserDelegationKeyResult`.
- Removed `DataLake` from the names of return types and option types.
- Removed `RequestId` from the return types.
- Changed `BodyStream` parameter of `Append` function from pointer to reference.
- Removed `PathRenameMode`, `PathGetPropertiesAction`, `PathSetAccessControlRecursiveMode`, `FileSystemResourceType`, `PathExpiryOptions` and `FileSystemResourceType`.
- Removed `IsAccessTierInferred` and `AccessTierChangedOn` from `PathProperties`.
- Renamed `LeaseDurationType` to `LeaseDuration`, `LeaseStateType` to `LeaseState` and `LeaseStatusType` to `LeaseStatus`.

## 12.0.0-beta.8 (2021-02-12)

### Breaking Changes

- Removed `DataLakeFileSystemClient::GetPathClient`.
- Renamed `SetDataLakePathAccessControlRecursiveListSinglePageOptions::MaxEntries` to `PageSizeHint`.
- `GetDataLakePathPropertiesResult::ServerEncrypted` was renamed to `IsServerEncrypted`.
- `GetDataLakePathPropertiesResult::AccessTierInferred` was renamed to `IsAccessTierInferred`.
- `HttpHeaders` of `DownloadDataLakeFileResult` and `DownloadDataLakeFileToResult` was moved into `Details`, to align with Blob service.
- Removed `BreakDataLakeLeaseResult::LeaseTime`.
- Renamed APIs for modifying access list recursively. Used to be with pattern `AccessControlRecursiveList`, now is with pattern `AccessControlListRecursive`.
- Refined options for `ScheduleDeletion`, to be consistent with other APIs.
- Renamed `ContentLength` in `PathItem` to `FileSize`.
- In `PathSetAccessControlRecursiveResult`, `DirectoriesSuccessful` is renamed to `NumberOfSuccessfulDirectories`, `FilesSuccessful` is renamed to `NumberOfSuccessfulFiles`, `FailureCount` is renamed to `NumberOfFailures`.
- Moved `Azure::Core::Context` out of options bag of each API, and make it the last optional parameter.

## 12.0.0-beta.7 (2021-02-03)

### New Features

- Added `Owner`, `Permissions`, and `Group` to `GetDataLakePathAccessControlResult`.
- `DownloadDataLakeFileResult` now has a new field `FileSize`.
- Added support for `GetAccessPolicy` and `SetAccessPolicy` in `DataLakeFileSystemClient`.
- Moved all protocol layer generated result types to `Details` namespace.
- Renamed `FileSystem` type returned from `ListDataLakeFileSystems` to be `FileSystemItem`. Member object name `FileSystems` is renamed to `Items`.
- Renamed `Path` type returned from `ListDataLakePaths` to be `PathItem`. Member object name `Paths` is renamed to `Items`.
- Added support for specifying public access type when creating a file system.
- Added `DataLakeDirectoryClient::ListPathsSinglePage` API to list DataLake paths under certain directory.
- Added `Metadata`, `AccessType`, `HasImmutabilityPolicy`, `HasLegalHold`, `LeaseDuration`, `LeaseState` and `LeaseStatus` to `FileSystemItem`.
- Added new type `LeaseDurationType` to indicate if a lease duration is fixed or infinite.
- Added `RequestId` in each return type for REST API calls, except for concurrent APIs.
- Added `UpdateAccessControlListRecursiveSinglePage` to update the access control recursively for a datalake path.
- Added `RemoveAccessControlListRecursiveSinglePage` to remove the access control recursively for a datalake path.

### Breaking Changes

- Removed `GetDfsUri` in all clients since they are currently implementation details.
- Removed `Data` suffix for `FlushData` and `AppendData` and modified all related structs to align the change.
- `DataLakePathClient` can no longer set permissions with `SetAccessControl`, instead, a new API `SetPermissions` is created for such functionality. Renamed the original API to `SetAccessControlList` to be more precise.
- `ContentRange` in `DownloadDataLakeFileResult` is now `Azure::Core::Http::Range`.
- Removed `ContentRange` in `PathGetPropertiesResult`.
- Renamed `ContentLength` in `GetDataLakePathPropertiesResult` and `CreateDataLakePathResult` to `FileSize` to be more accurate.
- Renamed `GetUri` to `GetUrl`.
- Added `DataLakeLeaseClient`, all lease related APIs are moved to `DataLakeLeaseClient`.
- Changed lease duration to be `std::chrono::seconds`.
- Removed `Directory` in `ListPathsSinglePageOptions`.
- Removed unused type `AccountResourceType` and `PathLeaseAction`.
- Changed all previous `LeaseDuration` members to a new type named `LeaseDurationType`.
- `startsOn` parameter for `GetUserDelegationKey` was changed to optional.
- Removed `PreviousContinuationToken` from `ListFileSystemsSinglePageResult`.
- `Concurrency`, `ChunkSize` and `InitialChunkSize` were moved into `DownloadDataLakeFileToOptions::TansferOptions`.
- `Concurrency`, `ChunkSize` and `SingleUploadThreshold` were moved into `UploadDataLakeFileFromOptions::TransferOptions`.
- Removed `Rename` from `DataLakeDirectoryClient` and `DataLakeFileClient`. Instead, added `RenameFile` and `RenameSubdirectory` to `DataLakeDirectoryClient` and added `RenameFile` and `RenameDirectory` to `DataLakeFileSystemClient`.
- Rename APIs now return the client of the resource it is renaming to.
- Removed `Mode` for rename operations' options, that originally controls the rename mode. Now it is fixed to legacy mode.
- Changed `SetAccessControlRecursive` to `SetAccessControlRecursiveListSinglePage`, to mark that it is a single page operation, and removed the `mode` parameter, separated the modify/delete functionality to two new APIs.
- Moved `SetAccessControlRecursiveListSinglePage` to `DataLakePathClient`.
- Changed `MaxRecord` to `MaxEntries`, `ForceFlag` to `ContinueOnFailure` to be more accurate names.
- Type for ETag was changed to `Azure::Core::ETag`.
- Type for `IfMatch` and `IfNoneMatch` was changed to `Azure::Core::ETag`.
- Renamed `ListDataLakeFileSystemsIncludeItem` to `ListDataLakeFileSystemsIncludeFlags`.
- Removed `DataLakeDirectoryClient::Delete` and `DataLakeDirectoryClient::DeleteIfExists`. Added `DataLakeDirectoryClient::DeleteEmpty`, `DataLakeDirectoryClient::DeleteEmptyIfExists`, `DataLakeDirectoryClient::DeleteRecursive` and `DataLakeDirectoryClient::DeleteRecursiveIfExists` instead.
- Removed `ContinuationToken` in `DeleteDataLakePathResult` and `DeleteDataLakeDirectoryResult`, as they will never be returned for HNS enabled accounts.
- Renamed `DataLakeFileClient::Read` to `DataLakeFileClient::Download`. Also changed the member `Azure::Core::Nullable<bool> RangeGetContentMd5` in the option to be `Azure::Core::Nullable<HashAlgorithm> RangeHashAlgorithm` instead.
- Moved some less commonly used properties into a details data structure for `Download`, `DownloadTo` and `ListFileSystemsSinglePage` API, and enriched the content of the mentioned details data structure.
 
### Other Changes and Improvements

- Changed `DataLakeFileClient::Flush`'s `endingOffset` parameter's name to `position`.
- Renamed `DataLakePathClient::GetAccessControls` to `DataLakePathClient::GetAccessControlList`.
- Removed unused parameters, options, results and functions in protocol layer.

## 12.0.0-beta.6 (2021-01-14)

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
