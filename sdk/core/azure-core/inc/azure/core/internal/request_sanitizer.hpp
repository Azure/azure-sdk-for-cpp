// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/url.hpp"
#include <string>

namespace Azure { namespace Core { namespace _internal {
  class InputSanitizer final {

  public:
    static Url SanitizeUrl(Url const& url);
  };
}}} // namespace Azure::Core::_internal
