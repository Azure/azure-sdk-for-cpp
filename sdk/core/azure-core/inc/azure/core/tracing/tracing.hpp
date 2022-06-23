// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Public TracerProvider type used to represent a tracer provider.
 */

#pragma once

#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Tracing {
  class TracerProvider;
  namespace _internal {
    class Tracer;
    /**
     * @brief Trace Provider - factory for creating Tracer objects.
     */
    class TracerProviderImpl {
    public:
      /**
       * @brief Create a Tracer object
       *
       * @param name Name of the tracer object, typically the name of the Service client
       * (Azure.Storage.Blobs, for example)
       * @param version Version of the service client.
       * @return std::shared_ptr<Azure::Core::Tracing::Tracer>
       */
      virtual std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
          std::string const& name,
          std::string const& version) const = 0;
    };

    std::shared_ptr<TracerProviderImpl> TracerImplFromTracer(
        std::shared_ptr<TracerProvider> const&);

  } // namespace _internal

  /**
   * @brief Trace Provider - factory for creating Tracer objects.
   */
  class TracerProvider : _internal::TracerProviderImpl {
    friend std::shared_ptr<TracerProviderImpl> _internal::TracerImplFromTracer(
        std::shared_ptr<TracerProvider> const&);
  };

}}} // namespace Azure::Core::Tracing
