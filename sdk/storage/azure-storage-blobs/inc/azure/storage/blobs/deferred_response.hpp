// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <vector>

#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  template <typename T> class DeferredResponse;

  namespace _detail {

    class DeferredResponseSharedBase {
    public:
      virtual ~DeferredResponseSharedBase() = 0;
    };

    template <typename T> class DeferredResponseShared : public DeferredResponseSharedBase {
    public:
      ~DeferredResponseShared() override {}

      virtual Response<T> GetResponse() = 0;
    };

    class DeferredResponseFactory {
    protected:
      DeferredResponseFactory() = default;
      DeferredResponseFactory(const DeferredResponseFactory&) = delete;
      DeferredResponseFactory(DeferredResponseFactory&&) = default;
      DeferredResponseFactory& operator=(const DeferredResponseFactory&) = delete;
      DeferredResponseFactory& operator=(DeferredResponseFactory&&) = default;

      template <typename T>
      DeferredResponse<T> CreateDeferredResponse(
          std::shared_ptr<DeferredResponseShared<T>> deferredOperationShared)
      {
        m_deferredOperations.push_back(deferredOperationShared);
        return DeferredResponse<T>(std::move(deferredOperationShared));
      }

      const std::vector<std::shared_ptr<DeferredResponseSharedBase>>& DeferredOperations() const
      {
        return m_deferredOperations;
      }

    private:
      std::vector<std::shared_ptr<DeferredResponseSharedBase>> m_deferredOperations;
    };

  } // namespace _detail

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
    Response<T> GetResponse() const { return m_sharedState->GetResponse(); }

  private:
    DeferredResponse(std::shared_ptr<_detail::DeferredResponseShared<T>> sharedState)
        : m_sharedState(std::move(sharedState))
    {
    }

  private:
    std::shared_ptr<_detail::DeferredResponseShared<T>> m_sharedState;

    friend class _detail::DeferredResponseFactory;
  };

}}} // namespace Azure::Storage::Blobs

