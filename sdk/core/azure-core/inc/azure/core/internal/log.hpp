// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/logging/logging.hpp"

namespace Azure { namespace Core { namespace Logging { namespace Internal {
  bool ShouldLog(LogLevel level);
  void Log(LogLevel level, std::string const& message);
}}}} // namespace Azure::Core::Logging::Internal
