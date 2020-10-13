# Release History

## 1.0.0-beta.3 (2020-10-13)

* Service version is now 2020-02-10.
* Added support for SAS generation in DataLake service.
* CreateFromConnectionString now accepts unencoded file, path and directory name.
* Added support for `FileClient::ScheduleDeletion`.
* Added support for `DirectoryClient::SetAccessControlRecursive`.
* `ETag` and `LastModified` is now `std::string` instead of `Azure::Core::Nullable<std::string>` in `CreateDirectoryResult`, `CreateFileResult` and `CreatePathResult`.
* `PathAppendDataResult` now returns `ContentMD5`, `ContentCrc64` and `IsServerEncrypted`.

## 1.0.0-beta.2 (2020-09-09)

* Release based on azure-core_1.0.0-beta.1

## 1.0.0-beta.1

* Support for Lease related operations.

## 1.0.0-preview.1 (Unreleased)

* Added support for DataLake features:
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
