// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>

namespace Azure { namespace Identity { namespace _detail {
  class ProcessHelper final {
    ProcessHelper() = delete;
    ~ProcessHelper() = delete;

  public:
    static std::string ExecuteProcess(
        std::string const& executable,
        std::string const& arguments,
        Core::Context const& context,
        DateTime::duration timeout);
  };
}}} // namespace Azure::Identity::_detail
