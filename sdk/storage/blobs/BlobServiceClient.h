// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <chrono>

#include "azure/core/Pipeline.h"
#include "azure/core/Uri.h"

#include "azure/storage/common/Lease.h"
#include "azure/storage/common/StorageSharedKeyCredential.h"

#include "models/BlobClientOptions.h"
#include "models/CreateBlobContainerOptions.h"
#include "models/DeleteBlobContainerOptions.h"
#include "models/GetUserDelegationKeyOptions.h"
#include "models/ListBlobContainersOptions.h"

namespace Azure { namespace Storage {

class BlobServiceClient
{
public:
    // Connection string
    BlobServiceClient(const std::string& connectionString, BlobClientOptions options = BlobClientOptions());

    // Shared_key auth
    BlobServiceClient(
        Azure::Core::Http::Uri uri,
        StorageSharedKeyCredential credential,
        BlobClientOptions options = BlobClientOptions();
        );

    // Token auth
    BlobServiceClient(
        Azure::Core::Http::Uri uri,
        Azure::Core::TokenCredential credential,
        BlobClientOptions options = BlobClientOptions();
        );

    // Anonymous/SAS/Customized pipeline auth
    BlobServiceClient(Azure::Core::Http::Uri serviceUri, BlobClientOptions options = BlobClientOptions());

    BlobContainerClient GetBlobContainerClient(std::string containerName);

    Azure::Core::Iterable<BlobContainerItem> ListBlobContainers(
        const ListBlobContainersOptions& options = ListBlobContainersOptions();
        ) const;

    Azure::Core::Http::Response<UserDelegationKey> GetUserDelegationKey(
        const std::chrono::time_point& endTime,
        const GetUserDelegationKeyOptions& options = GetUserDelegationKeyOptions();
        ) const;

    Azure::Core::Http::Response<BlobContainerClient> CreateBlobContainer(
        const std::string& BlobContainerName,
        const CreateBlobContainerOptions& options = CreateBlobContainerOptions()
        ) const;

    Azure::Core::Http::Response<void> DeleteBlobContainer(
        const std::string& BlobContainerName,
        const DeleteBlobContainerOptions& options = DeleteBlobContainerOptions()
        ) const;

private:
    Azure::Core::Http::Uri mServiceUri;
    Azure::Core::Pipeline mPipeline;
    std::string mAccountName;
};

}}
