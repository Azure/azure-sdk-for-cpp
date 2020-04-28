// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/Context.h"

namespace Azure { namespace Storage {

struct StorageRequestOptions
{
    Azure::Core::Context context = Azure::Core::Context::None();
};

}}
