// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace Azure { namespace Core { namespace _internal {
  class Environment final {
  private:
    Environment() = delete;
    ~Environment() = delete;

  public:
    static std::string GetVariable(const std::string& name) { return GetVariable(name.c_str()); }
    static void SetVariable(const std::string& name, const std::string& value)
    {
      SetVariable(name.c_str(), value.c_str());
    }

    static std::string GetVariable(const char* name);
    static void SetVariable(const char* name, const char* value);
  };
}}} // namespace Azure::Core::_internal
