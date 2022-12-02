# Release History

## 12.7.0-beta.1 (Unreleased)

### Features Added

### Breaking Changes

### Bugs Fixed

### Other Changes

## 12.6.2 (2022-10-11)

### Other Changes

- No public changes in this release.

## 12.6.1 (2022-09-16)

### Other changes

- No public changes in this release.

## 12.6.0 (2022-09-06)

### Features Added

- Features in `12.6.0.beta.1` are now generally available.

### Bugs Fixed

- Empty file or existing file won't be created/overwritten if the blob to be downloaded doesn't exist.

## 12.6.0-beta.1 (2022-08-09)

### Features Added

- Bumped up API version to `2021-04-10`.
- Added support for encryption scope SAS (`ses` query parameter in SAS token).
- Added encryption scope support for `BlobClient::CopyFromUri()`.
- Added support for filtering blobs on container level.
- Added support for tags copy mode (replace or copy from source) when copying blobs from URL.
- Added support for permanent delete permission in SAS.

### Bugs Fixed

- Fixed listing blobs failure when blob name contains invalid characters in xml.

## 12.5.0 (2022-08-08)

### Features Added

- Features in `12.5.0.beta1` and `12.5.0.beta2` are now generally available.

## 12.5.0-beta.2 (2022-07-07)

### Features Added

- Added support for Blob Batch.

## 12.5.0-beta.1 (2022-06-07)

### Features Added

- Bumped up API version to `2020-10-02`.
- Added new API: `BlockBlobClient::Query()`.
- Added `ContinuationToken` and `PageSizeHint` in `GetPageRangesOptions`.
- Added support for listing system containers.
- Added support for listing deleted root blob with active versions.

## 12.4.0 (2022-04-06)

### Other changes

- No public changes in this release.

## 12.4.0-beta.1 (2022-03-09)

### Features Added

- Bumped up API version to `2020-08-04`.
- Bumped up SAS token version to `2020-08-04`.
- Added support for immutability policy.
- New API: `BlobClient::CopyFromUri()`, `BlobClient::SetImmutabilityPolicy()`, `BlobClient::DeleteImmutabilityPolicy()`, `BlobClient::SetLegalHold()`, `BlockBlobClient::UploadFromUri()` and `BlobServiceClient::RenameBlobContainer()`.
- Added lease access condition support for `BlobClient::SetTags()` and `BlobClient::GetTags()`.

## 12.3.0 (2022-02-14)

### Features Added

- Added `VersionId` in `SetBlobMetadataResult`.

## 12.2.1 (2021-11-08)

### Other Changes

- `DefaultEncryptionScope` and `PreventEncryptionScopeOverride` in `BlobContainerProperties` were changed to optional.

## 12.2.0 (2021-09-08)

### Breaking Changes

- `StartsOn` and `ExpiresOn` in `SignedIdentifier` were changed to nullable.

### Bugs Fixed

- Fixed a bug where prefix cannot contain `&` when listing blobs.

### Other Changes

- Create less threads if there isn't too much data to transfer.

## 12.1.0 (2021-08-10)

### Features Added

- Added lease ID access condition and tags access condition for `BlobClient::SetAccessTier()`.
- Added source ETag access conditions and last-modified access conditions for `PageBlobClient::UploadPagesFromUri()`.
- Added three new fields `IsServerEncrypted`, `EncryptionKeySha256` and `EncryptionScope` into `SetBlobMetadataResult`.
- Added support for setting blob tags when creating or copying blobs.
- Added new fields `AccessTierChangedOn`, `ArchiveStatus`, `RehydratePriority`, `CopyId`, `CopySource`, `CopyStatus`, `CopyStatusDescription`, `IsIncrementalCopy`, `IncrementalCopyDestinationSnapshot`, `CopyProgress`, `CopyCompletedOn`, `Tags`, `DeletedOn` and `RemainingRetentionDays` into `BlobItemDetails`.
- Added support for including blob tags when listing blobs.

### Bugs Fixed

- Fixed a bug where lease ID didn't work for `BlobContainerClient::GetAccessPolicy()`.
- Fixed a bug where sequence number access conditions didn't work for page blob operations.
- Fixed a bug where `BlobItemDetails::EncryptionKeySha256` was always null because it wasn't correctly parsed from xml.
- Fixed a bug where `BlockBlobClient::GetBlockList` threw exception if the blob is not committed.

## 12.0.1 (2021-07-07)

### Bug Fixes

- Fixed a bug where transactional MD5 hash was treated as blob MD5 hash when downloading partial blob.

## 12.0.0 (2021-06-08)

### Other Changes and Improvements

- Added and updated some samples.
- Fixed a read consistency issue.

## 12.0.0-beta.11 (2021-05-19)

### Breaking Changes

- Added `final` specifier to classes and structures that are are not expected to be inheritable at the moment.
- Renamed `HasMorePages()` in paged response to `HasPage()`.
- Default chunk size for concurrent upload was changed to nullable.
- `BlobLeaseClient::Change()` updates internal lease id.
- Removed `ContentType` from `GetBlockListResult`.
- Moved `GetPageRangesResult` to detail namespace.
- `BlobServiceClient::UndeleteBlobContainer` doesn't support restoring a deleted container under a different name anymore.
- Changed the type of block count to `int32_t`.

## 12.0.0-beta.10 (2021-04-16)

### Breaking Changes

- Removed `Azure::Storage::Blobs::PackageVersion`.
- Renamed `GetUserDelegationKeyOptions::startsOn` to `StartsOn`.
- Replaced all paginated collection functions that have the SinglePage suffix with pageable functions returning a `PagedResponse<T>`-derived type. The options are also renamed accordingly.
  - `BlobServiceClient::ListBlobContainers()`.
  - `BlobServiceClient::FindBlobsByTags()`.
  - `BlobContainerClient::ListBlobs()`.
  - `BlobContainerClient::ListBlobsByHierarchy()`.
  - `PageBlobClient::GetPageRanges()`.
  - `PageBlobClient::GetPageRangesDiff()`.
  - `PageBlobClient::GetManagedDiskPageRangesDiff()`.
- Renamed `FilterBlobItem` to `TaggedBlobItem`.
  - `FindBlobsByTags()` now returns `FindBlobsByTagsPagedResponse` and the field `FindBlobsByTagsSinglePageResult::Items` was renamed to `FindBlobsByTagsPagedResponse::TaggedBlobs`.

## 12.0.0-beta.9 (2021-03-23)

### New Features

- Added support for telemetry options.
- Added `Azure::Storage::Blobs::PackageVersion`.
- Added `ShareFileClient::GetRangeListDiff`.

### Breaking Changes

- Changed the return type of `StartCopyFromUri` and `StartCopyIncremental` API from a `Response<T>` to the particular `Operation<T>` type called `StartBlobCopyOperation` directly.
- String conversion functions of extensible enums were renamed from `Get()` to `ToString()`.
- Moved `SecondaryHostForRetryReads` out of retry options, now it's under `BlobClientOptions`.
- Changed the return types of the following APIs:
  - `BlobServiceClient::DeleteBlobContainer` now returns `DeleteBlobContainerResult`.
  - `BlobServiceClient::GetUserDelegationKey` now returns `UserDelegationKey`.
  - `BlobServiceClient::GetProperties` now returns `BlobServiceProperties`.
  - `BlobServiceClient::GetAccountInfo` now returns `AccountInfo`.
  - `BlobServiceClient::GetStatistics` now returns `ServiceStatistics`.
  - `BlobContainerClient::DeleteBlob` now returns `DeleteBlobResult`.
  - `BlobContainerClient::GetProperties` now returns `BlobContainerProperties`.
  - `BlobContainerClient::GetAccessPolicy` now returns `BlobContainerAccessPolicy`.
  - `BlobClient::GetProperties` now returns `BlobProperties`.
  - `BlobClinet::GetTags` now returns `std::map<std::string, std::string>`.
  - `BlobClient::AbortCopyFromUri` now returns `AbortBlobCopyFromUriResult`.
  - `PageBlobClient::UploadPages` now returns `UploadPagesResult`.
  - `PageBlobClient::UploadPagesFromUri` now returns `UploadPagesFromUriResult`.
  - `PageBlobClient::ClearPages` now returns `ClearPagesResult`.
  - `PageBlobClient::GetPageRanges`, `PageBlobClient::GetPageRangesDiff` and `PageBlobClient::GetManagedDiskPageRangesDiff` now return `GetPageRangesResult`.
- Renamed some options:
  - Renamed `UploadPageBlobPagesOptions` to `UploadPagesOptions`.
  - Renamed `UploadPageBlobPagesFromUriOptions` to `UploadPagesFromUriOptions`.
  - Renamed `ClearPageBlobPagesOptions` to `ClearPagesOptions`.
  - Renamed `GetPageBlobPageRangesOptions` to `GetPageRangesOptions`.
  - Renamed `StartCopyBlobFromUriOptions` to `StartBlobCopyFromUriOptions`.
  - Renamed `StartCopyPageBlobIncrementalOptions` to `StartBlobCopyIncrementalOptions`.
- Removed `Blob` from the name of lease options and return types, like `AcquireBlobLeaseOptions` was renamed to `AcquireLeaseOptions`.
- Removed `Blob` prefix from the name of structs `BlobGeoReplication`, `BlobLeaseStatus`, `BlobLeaseState`, `BlobLeaseDurationType`, `BlobAnalyticsLogging`, `BlobMetrics`, `BlobCorsRule`, `BlobRetentionPolicy`, `BlobStaticWebsite`, `BlobArchiveStatus` and `BlobGeoReplicationStatus`.
- Removed `RequestId` from return types.
- Changed `BodyStream` parameter of `UploadBlob`, `Upload`, `UploadPages` and `AppendBlock`functions from pointer to reference.
- Renamed access tier members in return types and options from `Tier` to `AccessTier`.
- Renamed `BlockListTypeOption` to `BlockListType`.

## 12.0.0-beta.8 (2021-02-12)

### Breaking Changes

- Removed `BreakBlobLeaseResult::Leasetime`.
- Moved `Azure::Core::Context` out of options bag of each API, and make it the last optional parameter.

## 12.0.0-beta.7 (2021-02-03)

### New Features

- Added `RequestId` in API return types.
- Added some new properties in `GetBlobPropertiesResult`, `DownloadBlobResult` and `DownloadBlobToResult`.
- Added `RangeHashAlgorithm` in `DownloadBlobOptions`.
- Added `UploadBlob` in `BlobContainerClient`.

### Breaking Changes

- `UserDelegationKey` was changed to a member of `GetUserDelegationKeyResult` rather than a typedef like before.
- `AccessType` in `CreateBlobContainerOptions` was changed to non-nullable.
- `ListType` in `GetBlockListOptions` was changed to non-nullable.
- Added `BlobLeaseClient`, all lease related APIs are moved to `BlobLeaseClient`.
- Type for lease duration in requests was changed to `std::chrono::seconds`, in response was changed to enum.
- `PublicAccessType::Private` was renamed to `PublicAccessType::None`.
- `startsOn` parameter for `GetUserDelegationKey` was changed to optional.
- Removed `IfUnmodifiedSince` from `SetBlobContainerMetadataOptions`.
- Return types of `BlobClient::StartCopyFromUri` and `PageBlobClient::StartCopyIncremental` were changed to `StartCopyBlobResult`, supporting poll operations.
- Fixed typo `Expiries` in model types.
- Removed `PreviousContinuationToken` from `ListBlobContainersSinglePageResult`, `ListBlobsByHierarchySinglePageResult` and `ListBlobsSinglePageResult`.
- `ListBlobContainersIncludeItem` was renamed to `ListBlobContainersIncludeFlags`.
- `ListBlobsIncludeItem` was renamed to `ListBlobsIncludeFlags`.
- `Concurrency`, `ChunkSize` and `InitialChunkSize` were moved into `DownloadBlobToOptions::TansferOptions`.
- `Concurrency`, `ChunkSize` and `SingleUploadThreshold` were moved into `UploadBlockBlobFromOptions::TransferOptions`.
- Removed `TagValue` from `FilterBlobItem`, removed `Where` from `FindBlobsByTagsSinglePageResult`.
- Type for ETag was changed to `Azure::Core::ETag`.
- Removed `BlobPrefix` struct, use `std::string` instead.
- Refined `BlobContainerItem`, `BlobItem`, `DownloadBlobResult` and `DownloadBlobToResult`.

## 12.0.0-beta.6 (2021-01-14)

### New Features

- Added `CreateIfNotExists` and `DeleteIfExists` for blob containers and blobs.
- Added `IsHierarchicalNamespaceEnabled` in `GetAccountInfoResult`.
- Added `PageBlobClient::GetPageRangesDiff` and `PageBlobClient::GetManagedDiskPageRangesDiff`.
- Added `CreateBlobContainer`, `DeleteBlobContainer`, `UndeleteBlobContainer` into `BlobServiceClient`.
- Added `DeleteBlob` to `BlobContainerClient`.
- Support setting blob SAS permission with a raw string.

### Breaking Changes

- Renamed `AppendBlobAccessConditions::MaxSize` to `IfMaxSizeLessThanOrEqual`.
- Renamed `AppendBlobAccessConditions::AppendPosition` to `IfAppendPositionEqual`.
- `BlobServiceProperties.DefaultServiceVersion` is now nullable.
- Renamed `DeleteBlobSubRequest::containerName` to `blobContainerName`.
- Renamed `SetBlobAccessTierSubRequest::containerName` to `blobContainerName`.
- Renamed `BlobSasBuilder::ContainerName` to `BlobContainerName`.
- Renamed `BlobSasResource::Container` to `BlobContainer`.
- Renamed `AccountSasResource::Container` to `BlobContainer`
- Renamed some structs:
  - `CreateContainerResult` to `CreateBlobContainerOptions`
  - `CreateContainerOptions` to `CreateBlobContainerOptions`
  - `DeleteContainerResult` to `DeleteBlobContainerResult`
  - `DeleteContainerOptions` to `DeleteBlobContainerOptions`
  - `GetContainerPropertiesResult` to `GetBlobContainerPropertiesResult`
  - `GetContainerPropertiesOptions` to `GetBlobContainerPropertiesOptions`
  - `SetContainerMetadataResult` to `SetBlobContainerMetadataResult`
  - `SetContainerMetadataOptions` to `SetBlobContainerMetadataOptions`
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
  - `ContainerAccessConditions` to `BlobContainerAccessConditions`
  - `ListContainersSegmentResult` to `ListBlobContainersSegmentResult`
  - `ListContainersSegmentOptions` to `ListBlobContainersSegmentOptions`
- API signature for `CommitBlockList` has changed. `BlockType` doesn't need to be specified anymore.
- `PageBlobClient::GetPageRanges` doesn't support getting difference between current blob and a snapshot anymore. Use `PageBlobClient::GetPageRangesDiff` instead.
- Moved Blob SAS into `Azure::Storage::Sas` namespace.
- Replaced all transactional content MD5/CRC64 with the `ContentHash` struct.
- `EncryptionKeySha256` is changed to binary (`std::vector<uint8_t>`).
- `ContentMd5` HTTP header is renamed to `ContentHash`, the type is also changed to `ContentHash`.
- `ServerEncrypted` fields are renamed to `IsServerEncrypted`, and changed to non-nullable type.
- Added `Is` prefix to bool variable names. Like `IsAccessTierInferred`, `IsDeleted`.
- `IsServerEncrypted`, `EncryptionKeySha256` and `EncryptionScope` are removed from `ClearPageBlobPagesResult`, since they are never returned from storage server.
- `ListBlobsFlatSegment` is renamed to `ListBlobsSinglePage`.
- `ListBlobsByHierarchySegment` is renamed to `ListBlobsByHierarchySinglePage`.
- `ListBlobContainersSegment` is renamed to `ListBlobContainersSinglePage`.
- `FindBlobsByTags` is renamed to `FindBlobsByTagsSinglePage`.
- `MaxResults` in list APIs are renamed to `PageSizeHint`.
- All date time related strings are now changed to `Azure::Core::DateTime` type.
- Replaced `std::pair<int64_t, int64_t>` with `Azure::Core::Http::Range` to denote blob ranges.
- Made version strings private by moving them into the `Details` namespace.
- Replaced scoped enums that don't support bitwise operations with extensible enum.
- Continuation token of result types are changed to nullable.
- Renamed `Models::DeleteSnapshotsOption::Only` to `OnlySnapshots`.
- Renamed `SourceConditions` in API options to `SourceAccessConditions`.
- Removed Blob Batch.
- `DownloadBlobResult::Content-Range` is changed to an `Azure::Core::Http::Range`, an extra field `BlobSize` is added.
- Removed `Undelete` from `BlobContainerClient`.
- `BlobRetentionPolicy::Enabled` is renamed to `BlobRetentionPolicy::IsEnabled`, `BlobStaticWebsite::Enabled` is renamed to `BlobStaticWebsite::IsEnabled`.
- Changed type for metadata to case-insensitive `std::map`.
- Changed parameter type for token credential from `Azure::Identity::ClientSecretCredential` to `Azure::Core::TokenCredential`.
- Renamed member function `GetUri` of client types to `GetUrl`.
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
- Add TagCount and ExpiryTime in the responses of getting properties and downloading blobs.

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

## 1.0.0-beta.1 (2020-08-28)

### New Features

- Added support for Blob features:
  - BlobServiceClient::ListBlobContainersSegment
  - BlobServiceClient::GetUserDelegationKey
  - BlobServiceClient::SetProperties
  - BlobServiceClient::GetProperties
  - BlobServiceClient::GetAccountInfo
  - BlobServiceClient::GetStatistics
  - BlobContainerClient::Create
  - BlobContainerClient::Delete
  - BlobContainerClient::GetProperties
  - BlobContainerClient::SetMetadata
  - BlobContainerClient::ListBlobsFlat
  - BlobContainerClient::ListBlobsByHierarchy
  - BlobContainerClient::Undelete
  - BlobContainerClient::GetAccessPolicy
  - BlobContainerClient::SetAccessPolicy
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
  - AppendBlobClient::Seal
  - PageBlobClient::Create
  - PageBlobClient::UploadPages
  - PageBlobClient::UploadPagesFromUri
  - PageBlobClient::ClearPages
  - PageBlobClient::Resize
  - PageBlobClient::GetPageRanges
  - PageBlobClient::StartCopyIncremental
- Support for blob versioning.
- Support for blob lease and container lease.
- Support for account SAS and blob SAS.
- Support for transactional checksum.