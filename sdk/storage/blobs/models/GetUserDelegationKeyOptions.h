// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

#include "BlobRequestOptions.h"

namespace Azure { namespace Storage {

struct GetUserDelegationKeyOptions : public BlobRequestOptions
{
    std::chrono::time_point startTime;
};

}}
