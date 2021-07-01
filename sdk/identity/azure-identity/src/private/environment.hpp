// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Identity { namespace _detail {
  class Environment final {
  private:
    Environment() = delete;
    ~Environment() = delete;

  public:
    static std::string GetVariable(const char* name);
  };
}}} // namespace Azure::Identity::_detail
