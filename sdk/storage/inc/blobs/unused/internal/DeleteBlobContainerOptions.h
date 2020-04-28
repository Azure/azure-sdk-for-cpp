// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "BlobLease.h"
#include "BlobRequestOptions.h"

namespace Azure { namespace Storage {

struct DeleteBlobContainerOptions : public BlobRequestOptions
{
    BlobLease lease;
};

}}
