// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

/**
 * @brief Add all non-optional headers from Azure Core.
 *
 * @remark The transport adapter headers are not included and are expected to be manually included.
 *
 */

// azure/core
#include "azure/core/context.hpp"
#include "azure/core/credentials.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "azure/core/strings.hpp"
#include "azure/core/uuid.hpp"
#include "azure/core/version.hpp"

// azure/core/http
#include "azure/core/http/body_stream.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/core/http/transport.hpp"

// azure/core/logging
#include "azure/core/logging/logging.hpp"
