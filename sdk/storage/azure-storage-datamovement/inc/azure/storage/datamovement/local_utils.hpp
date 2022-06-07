// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Storage { namespace _internal {
  class Local_Utils final {
  public:
    static void create_directory(std::string& path);
  };
}}} // namespace Azure::Storage::_internal