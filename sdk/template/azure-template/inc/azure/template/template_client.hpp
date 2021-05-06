// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Template {

  class TemplateClient {
  public:
    std::string ClientVersion() const;
  };

}} // namespace Azure::Template
