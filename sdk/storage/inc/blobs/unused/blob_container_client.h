// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage {

class BlobContainerClient
{
public:
    // connection string
    BlobContainerClient(const std::string& connectionString, std::string containerName, BlobContainerClientOptions options = BlobContainerClientOptions());

    // shared key auth
    BlobContainerClient(Azure::Core::Http::Uri uri, Azure::Storage::Common::StorageSharedKeyCredential credential, BlobContainerClientOptions options = BlobContainerClientOptions());

    // token auth
    BlobContainerClient(Azure::Core::Http::Uri uri, Azure::Core::TokenCredential credential, BlobContainerClientOptions options = BlobContainerClientOptions());

    // anonymous/SAS/customized pipeline auth
    BlobContainerClient(Azure::Core::Http::Uri uri, BlobContainerClientOptions options = BlobContainerClientOptions());

    BlobClient GetBlobClient(const std::string& blobName);

    Azure::Core::Http::Response<BlobContainerInfo> Create(const CreateBlobContainerOptions& options = CreateBlobContainerOptions());

    Azure::Core::Http::Response<void> Delete(const DeleteBlobContainerOptions& options = DeleteBlobContaienrOptions());

    Azure::Core::Http::Response<bool> Exists(const BlobContainerExistsOptions& options = BlobContainerExistsOptions());

    Azure::Core::Http::Response<BlobContainerProperties> GetProperties(const GetBlobContainerPropertiesOptions& options = GetBlobContainerPropertiesOptions());

    Azure::Core::Http::Response<void> SetHttpHeaders(const BlobContainerHttpHeaders& headers, SetBlobContainerHttpHeadersOptions options = SetBlobContainerHttpHeadersOptions());

    Azure::Core::Http::Response<void> SetMetadata(const std::map<std::string, std::string>& metadata, const SetBlobContainerMetadataOptions& options = SetBlobContainerMetadataOptions());

    Azure::Core::Iterable<BlobItem> ListBlobs(const ListBlobsOptions& options = ListBlobsOptions());

private:
    Azure::Core::Http::Uri mContainerUri;
    Azure::Core::Pipeline mPipeline;
    std::string mContainerName;
};

}}
