// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "BlobRequestOptions.h"

namespace Azure { namespace Storage {

struct ListBlobContainersOptions : public BlobRequestOptions
{
    std::string prefix;
    std::string continuationToken;
    uint16_t maximumResult = 0, // Caution, when value is 0, the header should not be sent to server
    bool includeMetadata = true,
};

}}
