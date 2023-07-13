// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/tracing/service_tracing.hpp>

#include <string>

namespace Azure { namespace Template {

  struct TemplateClientOptions : public Azure::Core::_internal::ClientOptions
  {
  };

  /** @brief The Azure Template service client. */
  class TemplateClient final {

  public:
    /** @brief Construct a new TemplateClient object.
     *
     * @param options Optional client options.
     */
    TemplateClient(TemplateClientOptions const& options = TemplateClientOptions{});
    /** @brief Return the value associated with the input key.
     *
     * @param key Key to query.
     * @param context Context for cancelling long running operations.
     * @returns the value associated with the key.
     */
    int GetValue(int key, Azure::Core::Context const& context = Azure::Core::Context{}) const;

  private:
    Azure::Core::Tracing::_internal::TracingContextFactory m_tracingFactory;
  };

}} // namespace Azure::Template
