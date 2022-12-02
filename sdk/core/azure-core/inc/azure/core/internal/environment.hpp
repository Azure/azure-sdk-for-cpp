// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Core { namespace _internal {
  class Environment final {
  private:
    Environment() = delete;
    ~Environment() = delete;

  public:
    static std::string GetVariable(const char* name);
    static void SetVariable(const char* name, const char* value);
  };
}}} // namespace Azure::Core::_internal