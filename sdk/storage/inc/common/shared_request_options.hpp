// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "nullable.hpp"

namespace Azure { namespace Storage {

  struct SharedRequestOptions
  {
    /**
     * @brief An optional operation timeout value in seconds. The period begins
     *        when the request is received by the service. If the timeout value
     *        elapses before the operation completes, the operation fails.
     */
    Azure::Core::Nullable<int32_t> Timeout;

    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;
  };
}} // namespace Azure::Storage
