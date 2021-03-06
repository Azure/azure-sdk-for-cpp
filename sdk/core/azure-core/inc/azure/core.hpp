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
#include "azure/core/base64.hpp"
#include "azure/core/context.hpp"
#include "azure/core/credentials.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/dll_import_export.hpp"
#include "azure/core/exception.hpp"
#include "azure/core/logger.hpp"
#include "azure/core/match_conditions.hpp"
#include "azure/core/modified_conditions.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/operation.hpp"
#include "azure/core/operation_status.hpp"
#include "azure/core/platform.hpp"
#include "azure/core/uuid.hpp"
#include "azure/core/version.hpp"
#include "azure/etag.hpp"

// azure/core/cryptography
#include "azure/core/cryptography/hash.hpp"

// azure/core/http
#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/core/http/transport.hpp"

// azure/core/io
#include "azure/core/io/body_stream.hpp"
