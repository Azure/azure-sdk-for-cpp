// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/logger.hpp"

namespace Azure { namespace Core { namespace Internal {

  inline bool ShouldLog(LogLevel level) { return Logger::GetListener(level) != nullptr; }

  inline void Log(LogLevel level, std::string const& message)
  {
    if (auto listener = Logger::GetListener(level))
    {
      listener(level, message);
    }
  }

}}} // namespace Azure::Core::Internal
