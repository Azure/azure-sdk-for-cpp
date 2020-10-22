# Release History

## 1.0.0-beta.5 (Unreleased)

### Breaking Changes
* `Azure::Storage::Files::Shares::Metrics::IncludeAPIs` is now renamed to `Azure::Storage::Files::Shares::Metrics::IncludeApis`, and is changed to a nullable member.
* Move header `azure/storage/files/shares/shares.hpp` to `azure/storage/files/shares.hpp`


## 1.0.0-beta.4 (2020-10-16)

### Bug Fixes

* Unencoded Share/File/Directory name is now encoded.

## 1.0.0-beta.3 (2020-10-13)

### New Features

* Service version is now 2020-02-10.
* Added support for leasing a share:
  - ShareClient::AcquireLease
  - ShareClient::ReleaseLease
  - ShareClient::ChangeLease
  - ShareClient::BreakLease
  - ShareClient::RenewLease

### Breaking Changes

* `CreateFromConnectionString` now accepts unencoded file and directory name.
* Added support for getting range list with previous snapshot. `GetFileRangeListResult` now returns `std::vector<FileRange> Ranges` and `std::vector<FileRange> ClearRanges` instead of `std::vector<Range> RangeList`.
* Added support for SMB Multi-Channel setting for `ServiceClient::GetProperties` and `ServiceClient::SetProperties`. This is only available for Storage account with Premium File access.
  - Standard account user has to remove the returned SMB Multi-Channel setting before set, otherwise service would return failure.
* `Marker` is renamed to `ContinuationToken` in options.
* `NextMarker` is renamed to `ContinuationToken` in returned result objects.
* `Marker` is renamed to `PreviousContinuationToken` in returned result objects.

## 1.0.0-beta.2 (2020-09-09)

### New Features

* Added File SAS generation support.
* Release based on azure-core_1.0.0-beta.1

## 1.0.0-preview.1 (Unreleased)

### New Features

* Added support for File features:
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
