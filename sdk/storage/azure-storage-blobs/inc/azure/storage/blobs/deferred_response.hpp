// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include <azure/core/response.hpp>

namespace Azure { namespace Storage {
  namespace Blobs {
    class BlobBatch;
  }
  /**
   * @brief Base type for a deferred response.
   */
  template <typename T> class DeferredResponse final {
  public:
    DeferredResponse(const DeferredResponse&) = delete;
    DeferredResponse(DeferredResponse&&) = default;
    DeferredResponse& operator=(const DeferredResponse&) = delete;
    DeferredResponse& operator=(DeferredResponse&&) = default;

    /**
     * @brief Gets the deferred response.
     *
     * @remark It's undefined behavior to call this function before the response or exception is
     * available.
     *
     * @return The deferred response. An exception is thrown if error occurred.
     */
    Response<T> GetResponse() const { return m_func(); }

  private:
    DeferredResponse(std::function<Response<T>()> func) : m_func(std::move(func)) {}

  private:
    std::function<Response<T>()> m_func;

    friend class Blobs::BlobBatch;
  };
}} // namespace Azure::Storage
