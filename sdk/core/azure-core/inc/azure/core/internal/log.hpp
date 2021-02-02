// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/logging/logging.hpp"

namespace Azure { namespace Core { namespace Logging { namespace Internal {
  bool ShouldWrite(LogLevel level);
  void Write(LogLevel level, std::string const& message);
}}}} // namespace Azure::Core::Logging::Internal
