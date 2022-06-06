// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/tracing/service_tracing.hpp>
#include <string>

namespace Azure { namespace Template {

  struct TemplateClientOptions : public Azure::Core::_internal::ClientOptions
  {
  };

  class TemplateClient final {
  public:
    TemplateClient(TemplateClientOptions options = TemplateClientOptions());
 
    int GetValue(int p1, int p2, int d) const;
  };

}} // namespace Azure::Template
