// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>

#include "BlobRequestOptions.h"
#include "BlobContainerAccessType.h"

namespace Azure { namespace Storage {

struct CreateBlobContainerOptions : public BlobRequestOptions
{
    BlobContainerAccessType accessType;
    std::map<std::string, std::string> metadata;
};

}}
