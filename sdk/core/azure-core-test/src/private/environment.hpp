// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/test/network_models.hpp>

#include <string>

namespace Azure { namespace Core { namespace Test { namespace _detail {
  class Environment final {
  private:
    Environment() = delete;
    ~Environment() = delete;

  public:
    static std::string GetVariable(const char* name);
    static Azure::Core::Test::TestMode GetTestMode();
  };
}}}} // namespace Azure::Core::Test::_detail
