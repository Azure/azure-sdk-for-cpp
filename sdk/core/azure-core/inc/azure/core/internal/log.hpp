// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/logger.hpp"

namespace Azure { namespace Core {
  namespace Details {
    Logger::Listener GetLogListener(Logger::Level);
  }

  namespace Internal {

    inline bool ShouldLog(Logger::Level level) { return Details::GetLogListener(level) != nullptr; }

    inline void Log(Logger::Level level, std::string const& message)
    {
      if (auto listener = Details::GetLogListener(level))
      {
        listener(level, message);
      }
    }

  } // namespace Internal
}} // namespace Azure::Core
