// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/internal/diagnostics/log.hpp>

namespace Azure { namespace Identity { namespace _detail {

  class IdentityLog final {
  public:
    using Level = Core::Diagnostics::Logger::Level;

    static void Write(Level level, std::string const& message)
    {
      Core::Diagnostics::_internal::Log::Write(level, "Identity: " + message);
    }

    static bool ShouldWrite(Level level)
    {
      return Core::Diagnostics::_internal::Log::ShouldWrite(level);
    }

  private:
    IdentityLog() = delete;
    ~IdentityLog() = delete;
  };

}}} // namespace Azure::Identity::_detail
