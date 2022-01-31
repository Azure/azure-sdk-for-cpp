// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/diagnostics/logger.hpp"

namespace Azure { namespace Core { namespace Diagnostics { namespace _detail {

  class EnvironmentLogLevelListener final {
    EnvironmentLogLevelListener() = delete;
    ~EnvironmentLogLevelListener() = delete;

  public:
    static Logger::Level GetLogLevel(Logger::Level defaultValue);
    static std::function<void(Logger::Level level, std::string const& message)> GetLogListener();
    static bool IsInitialized();
    static void SetInitialized(bool value);
  };

}}}} // namespace Azure::Core::Diagnostics::_detail
