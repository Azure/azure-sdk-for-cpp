// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage {

class BlobClient
{
public:
    // connection string
    BlobClient(const std::string& connectionString, std::string containerName, std::string blobName, BlobClientOptions options = BlobClientOptions());

    // shared key auth
    BlobClient(Azure::Core::Http::Uri uri, Azure::Storage::Common::StorageSharedKeyCredential credential, BlobClientOptions options = BlobClientOptions());

    // token auth
    BlobClient(Azure::Core::Http::Uri uri, Azure::Core::TokenCredential credential, BlobClientOptions options = BlobClientOptions());

    // anonymous/SAS/customized pipeline auth
    BlobClient(Azure::Core::Http::Uri uri, BlobClientOptions options = BlobClientOptions());

    void WithSnapshot(const std::string& snapshot);
    const std::string& Snapshot() const;

    void WithVersionId(const std::string& versionId);
    const std::string& VersionId() const;

    Azure::Storage::Http::Response<void> Delete(const DeleteBlobOptions& options = DeleteBlobOptions());

    Azure::Storage::Http::Response<bool> Exists(const BlobExistsOptions& options = BlobExistsOptions());

    Azure::Storage::Http::Response<BlobProperties> GetProperties(const GetBlobPropertiesOptions& options = GetBlobPropertiesOptions());

    Azure::Storage::Http::Response<void> SetHttpHeaders(const BlobHttpHeaders& headers, SetBlobHttpHeadersOptions options = SetBlobHttpHeadersOptions());

    Azure::Storage::Http::Response<void> SetMetadata(const std::map<std::string, std::string>& metadata, const SetBlobMetadataOptions = SetBlobMetadataOptions());

    Azure::Storage::Http::Response<BlobSnapshotInfo> CreateSnapshot(const CreateBlobSnapshotOptions options = CreateBlobSnapshotOptions());

    Azure::Storage::Http::Response<BlobDownloadInfo> Download(const DownloadBlobOptions& options = DownloadBlobOptions());

    // parallel download
    BlobDownloadInfo DownloadTo(std::ostream& destination, const DownloadBlobOptions& options = DownloadBlobOptions());
    BlobDownloadInfo DownloadTo(const std::string& path, const DownloadBlobOptions& options = DownloadBlobOptions());
    BlobDownloadInfo DownloadTo(char* buffer, uint64_t size, const DownloadBlobOptions& options = DownloadBlobOptions());

private:
    Azure::Core::Http::Uri mBlobUri;
    Azure::Core::Pipeline mPipeline;
    std::string mBlobName;
};

}}
