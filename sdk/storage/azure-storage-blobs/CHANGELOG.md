# Release History

## 1.0.0-beta.2

* Support for Blob Batch
* Support for Blob Index

## 1.0.0-beta.1

* New APIs:
  - BlobServiceClient::SetProperties
  - BlobServiceClient::GetProperties
  - BlobServiceClient::GetAccountInfo
  - BlobServiceClient::GetStatistics
  - BlobContainerClient::Undelete
  - BlobContainerClient::GetAccessPolicy
  - BlobContainerClient::SetAccessPolicy
  - AppendBlobClient::Seal
* Support for blob versioning
* Support for blob lease and container lease
* Support for account SAS and blob SAS
* Support for transactional checksum


## 1.0.0-preview.1 (Unreleased)

* Added support for Blob features:
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
