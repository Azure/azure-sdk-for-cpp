// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/internal/client_options.hpp>
// #include <azure/core/internal/tracing/service_tracing.hpp>
#include <string>

namespace Azure { namespace Template {

  struct TemplateClientOptions : public Azure::Core::_internal::ClientOptions
  {
  };
  class TemplateClient final {

  public:
    TemplateClient(TemplateClientOptions const& options = TemplateClientOptions{});
    int GetValue(int key, Azure::Core::Context const& context = Azure::Core::Context{}) const;

  private:
    //Azure::Core::Tracing::_internal::TracingContextFactory m_tracingFactory;
  };

}} // namespace Azure::Template
