// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

/**
 * @file core.hpp
 * @brief Adds all headers from Azure Core.
 *
 */

// azure/core
#include "azure/core/azure.hpp"
#include "azure/core/context.hpp"
#include "azure/core/credentials.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/response.hpp"
#include "azure/core/uuid.hpp"
#include "azure/core/version.hpp"

// azure/core/http
#include "azure/core/http/body_stream.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/core/http/transport.hpp"

// azure/core/http/curl
#include "azure/core/http/curl/curl.hpp"

// azure/core/http/winhttp
#include "azure/core/http/winhttp/win_http_client.hpp"

// azure/core/logging
#include "azure/core/logging/logging.hpp"