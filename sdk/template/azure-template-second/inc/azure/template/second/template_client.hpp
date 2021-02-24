// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Template { namespace Second {

  class TemplateClient {
  public:
    std::string const ClientVersion();
  };

}}} // namespace Azure::Template::Second