// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Template {

  class TemplateClient final {
  public:
    std::string ClientVersion() const;
  };

}} // namespace Azure::Template
