// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/core/Uri.h"

namespace Azure { namespace Storage {

class BlobUriBuilder : public StorageUriBuilder
{
public:
    explicit BlobUriBuilder(Azure::Core::Http::Uri uri);

    const std::string& GetContainerName() const;
    void SetContainerName(std::string containerName);

    const std::string& GetBlobName() const;
    void SetBlobName(std::string blobName);

    Azure::Core::Http::Uri ToUri() const;
    std::string ToString() const;

private:
    std::string mBlobName;
    std::string mContainerName;
};

}}
