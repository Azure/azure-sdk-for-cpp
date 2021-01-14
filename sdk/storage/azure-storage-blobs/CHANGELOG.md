# Release History

## 12.0.0-beta.6 (2020-01-14)

### New Features

- `CreateIfNotExists` and `DeleteIfExists` for blob containers and blobs.
- Add `IsHierarchicalNamespaceEnabled` in `GetAccountInfoResult`.
- New API: `PageBlobClient::GetPageRangesDiff` and `PageBlobClient::GetManagedDiskPageRangesDiff`.
- Add `CreateBlobContainer`, `DeleteBlobContainer`, `UndeleteBlobContainer` into `BlobServiceClient`.
- Add `DeleteBlob` into `BlobContainerClient`.
- Support setting blob sas permission with a raw string.

### Breaking Changes

- Rename AppendBlobAccessConditions::MaxSize to AppendBlobAccessConditions::IfMaxSizeLessThanOrEqual.
- Rename AppendBlobAccessConditions::AppendPosition to AppendBlobAccessConditions::IfAppendPositionEqual.
- BlobServiceProperties.DefaultServiceVersion is now nullable.
- Rename `DeleteBlobSubRequest::containerName` to `DeleteBlobSubRequest::blobContainerName`.
- Rename `SetBlobAccessTierSubRequest::containerName` to `SetBlobAccessTierSubRequest::blobContainerName`.
- Rename `BlobSasBuilder::ContainerName` to `BlobSasBuilder::BlobContainerName`.
- Rename `BlobSasResource::Container` to `BlobSasResource::BlobContainer`.
- Rename `AccountSasResource::Container` to `AccountSasResource::BlobContainer`
- Rename some structs:
  - `CreateContainerResult` to `CreateBlobContainerOptions`
  - `CreateContainerOptions` to `CreateBlobContainerOptions`
  - `DeleteContainerResult` to `DeleteBlobContainerResult`
  - `DeleteContainerOptions` to `DeleteBlobContainerOptions`
  - `GetContainerPropertiesResult` to `GetBlobContainerPropertiesResult`
  - `GetContainerPropertiesOptions` to `GetBlobContainerPropertiesOptions`
  - `SetContainerMetadataResult` to `SetBlobContainerMetadataResult`
  - `SetContainerMetadataOptions ` to `SetBlobContainerMetadataOptions`
  - `GetContainerAccessPolicyResult` to `GetBlobContainerAccessPolicyResult`
  - `GetContainerAccessPolicyOptions` to `GetBlobContainerAccessPolicyOptions`
  - `SetContainerAccessPolicyResult` to `SetBlobContainerAccessPolicyResult`
  - `SetContainerAccessPolicyOptions` to `SetBlobContainerAccessPolicyOptions`
  - `AcquireContainerLeaseResult` to `AcquireBlobContainerLeaseResult`
  - `AcquireContainerLeaseOptions` to `AcquireBlobContainerLeaseOptions`
  - `RenewContainerLeaseResult` to `RenewBlobContainerLeaseResult`
  - `RenewContainerLeaseOptions` to `RenewBlobContainerLeaseOptions`
  - `ReleaseContainerLeaseResult` to `ReleaseBlobContainerLeaseResult`
  - `ReleaseContainerLeaseOptions` to `ReleaseBlobContainerLeaseOptions`
  - `ChangeContainerLeaseResult` to `ChangeBlobContainerLeaseResult`
  - `ChangeContainerLeaseOptions` to `ChangeBlobContainerLeaseOptions`
  - `BreakContainerLeaseResult` to `BreakBlobContainerLeaseResult`
  - `BreakContainerLeaseOptions` to `BreakBlobContainerLeaseOptions`
  - `ContainerAccessConditions ` to `BlobContainerAccessConditions`
  - `ListContainersSegmentResult` to `ListBlobContainersSegmentResult`
  - `ListContainersSegmentOptions` to `ListBlobContainersSegmentOptions`
- API signature for CommitBlockList has changed. `BlockType` doesn't need to be specified anymore.
- `PageBlobClient::GetPageRanges` doesn't support getting difference between current blob and a snapshot anymore. Use `PageBlobClient::GetPageRangesDiff` instead.
- Move Blob SAS into `Azure::Storage::Sas` namespace.
- Replace all transactional content MD5/CRC64 with `ContentHash` struct.
- `EncrytionKeySha256` is changed to binary(`std::vector<uint8_t>`).
- `ContentMd5` HTTP header is renamed to `ContentHash`, the type is also changed to `ContentHash`.
- `ServerEncrypted` fields are renamed to `IsServerEncrypted`, and changed to non-nullable type.
- Add `Is` prefix to bool variable names. Like `IsAccessTierInferred`, `IsDeleted`.
- `IsServerEncrypted`, `EncryptionKeySha256` and `EncryptionScope` are removed from `ClearPageBlobPagesResult`, since they are never returned from storage server.
- `ListBlobsFlatSegment` is renamed to `ListBlobsSinglePage`.
- `ListBlobsByHierarchySegment` is renamed to `ListBlobsByHierarchySinglePage`.
- `ListBlobContainersSegment` is renamed to `ListBlobContainersSinglePage`.
- `FindBlobsByTags` is renamed to `FindBlobsByTagsSinglePage`.
- `MaxResults` in list APIs are renamed to `PageSizeHint`.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- Replace `std::pair<int64_t, int64_t>` with `Azure::Core::Http::Range` to denote blob ranges.
- Move version strings into `Details` namespace.
- Replace scoped enums that don't support bitwise operations with extensible enum.
- Continuation token of result types are changed to nullable.
- Rename `Models::DeleteSnapshotsOption::Only` to `Models::DeleteSnapshotsOption::OnlySnapshots`.
- Rename `SourceConditions` in API options to `SourceAccessConditions`.
- Remove Blob Batch.
- `DownloadBlobResult::Content-Range` is changed to an `Azure::Core::Http::Range`, an extra field `BlobSize` is added.
- Remove `Undelete` from `BlobContainerClient`.
- `BlobRetentionPolicy::Enabled` is renamed to `BlobRetentionPolicy::IsEnabled`, `BlobStaticWebsite::Enabled` is renamed to `BlobStaticWebsite::IsEnabled`.
- Change type for metadata to case-insensitive `std::map`.
- Change parameter type for token credential from `Azure::Identity::ClientSecretCredential` to `Azure::Core::TokenCredential`.
- Rename member function `GetUri` of client types to `GetUrl`.
- `BlobClient::GetBlockBlobClient`, `BlobClient::GetAppendBlobClient` and `BlobClient::GetPageBlobClient` are renamed to `BlobClient::AsBlockBlobClient`, `BlobClient::AsAppendBlobClient` and `BlobClient::AsPageBlobClient` respectively.

## 12.0.0-beta.5 (2020-11-13)

### New Features

- Support for replaceable HTTP transport layer.
- Add `version.hpp`.

### Breaking Changes

- Move header `azure/storage/blobs/blob.hpp` to `azure/storage/blobs.hpp`.
- Service API return types which are typically suffixed with `Result` are moved to the `Models` sub-namespaces and everything else from the protocol layer is made private by moving to the `Details` namespace.
- Make XML serializer and deserializer private by moving them to the `Details` namespace.
- Remove `BlockBlobClientOptions`, `AppendBlobClientOptions` and `PageBlobClientOptions`, use `BlobClientOptions` instead.
- Rename `BlobSasBuilder::ToSasQueryParameters` to `BlobSasBuilder::GenerateSasToken`.

### Other Changes and Improvements

- Default uploading/downloading concurrency is changed from 1 to 5.
- Remove support for specifying SAS version.

## 1.0.0-beta.4 (2020-10-16)

### New Features

- Bump up API version to 2020-02-10.
- Support for Last Accessting Time.
- Add TagCount and ExiryTime in the responses of getting propertites and downloading blobs.

### Breaking Changes

- Variable name change: BreakContainerLeaseOptions::breakPeriod -> BreakContainerLeaseOptions::BreakPeriod.
- Variable name change: BreakBlobLeaseOptions::breakPeriod -> BreakBlobLeaseOptions::BreakPeriod.
- CreateFromConnectionString now accepts unencoded blob name.
- TagConditions is changed to nullable.
- Variable name change: `Marker` is renamed to `ContinuationToken` for `ListContainersSegmentOptions`, `FindBlobsByTagsOptions` and `ListBlobsSegmentOptions`.
- Variable name change: `Marker` is renamed to `PreviousContinuationToken`, `NextMarker` is renamed to `ContinuationToken` for `FilterBlobsSegmentResult`, `ListContainersSegmentResult`, `ListBlobsByHierarchySegmentResult` and `ListBlobsFlatSegmentResult`.

### Bug Fixes

- Unencoded Container/Blob name is now encoded.

## 1.0.0-beta.2 (2020-09-09)

### New Features

- Support for Blob Batch.
- Support for Blob Index.
- Release based on azure-core_1.0.0-beta.1.

## 1.0.0-beta.1

### New Features

- New APIs:
  - BlobServiceClient::SetProperties
  - BlobServiceClient::GetProperties
  - BlobServiceClient::GetAccountInfo
  - BlobServiceClient::GetStatistics
  - BlobContainerClient::Undelete
  - BlobContainerClient::GetAccessPolicy
  - BlobContainerClient::SetAccessPolicy
  - AppendBlobClient::Seal
- Support for blob versioning.
- Support for blob lease and container lease.
- Support for account SAS and blob SAS.
- Support for transactional checksum.


## 1.0.0-preview.1 (Unreleased)

### New Features

- Added support for Blob features:
  - BlobServiceClient::ListBlobContainersSegment
  - BlobServiceClient::GetUserDelegationKey
  - BlobContainerClient::Create
  - BlobContainerClient::Delete
  - BlobContainerClient::GetProperties
  - BlobContainerClient::SetMetadata
  - BlobContainerClient::ListBlobsFlat
  - BlobContainerClient::ListBlobsByHierarchy
  - BlobClient::GetProperties
  - BlobClient::SetHttpHeaders
  - BlobClient::SetMetadata
  - BlobClient::SetAccessTier
  - BlobClient::StartCopyFromUri
  - BlobClient::AbortCopyFromUri
  - BlobClient::Download
  - BlobClient::DownloadToFile
  - BlobClient::DownloadToBuffer
  - BlobClient::CreateSnapshot
  - BlobClient::Delete
  - BlobClient::Undelete
  - BlockBlobClient::Upload
  - BlockBlobClient::UploadFromFile
  - BlockBlobClient::UploadFromBuffer
  - BlockBlobClient::StageBlock
  - BlockBlobClient::StageBlockFromUri
  - BlockBlobClient::CommitBlockList
  - BlockBlobClient::GetBlockList
  - AppendBlobClient::Create
  - AppendBlobClient::AppendBlock
  - AppendBlobClient::AppendBlockFromUri
  - PageBlobClient::Create
  - PageBlobClient::UploadPages
  - PageBlobClient::UploadPagesFromUri
  - PageBlobClient::ClearPages
  - PageBlobClient::Resize
  - PageBlobClient::GetPageRanges
  - PageBlobClient::StartCopyIncremental
