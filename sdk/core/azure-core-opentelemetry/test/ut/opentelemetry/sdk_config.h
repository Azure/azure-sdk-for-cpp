// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

// The OpenTelemetry vcpkg package appears to be missing the file sdk_config.h, this file is a replacement
// to enable the use of in-memory exporters.
#pragma once

#include "opentelemetry/config.h"
#include "opentelemetry/sdk/common/global_log_handler.h"
