// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include <exception>
#include <functional>

namespace Azure { namespace Core {
  template <typename T> class DeferredResponse;

  namespace _internal {

    /**
     * @brief Base class for deferred operation shared state. Wraps the outgoing request.
     *
     * This class exists to allow DeferredResponseSharedWithCallback objects to be aggregated
     * where the specialization of the DeferredResponseSharedWithCallback has different
     * specialization types.
     *
     */
    class DeferredResponseSharedBase {
      friend class DeferredResponseFactory;

    public:
      Azure::Core::Http::Request m_request;

      virtual Azure::Core::Http::Request Request() { return m_request; }
      /**
       * @brief Process a raw response received from the service and save it for later
       * retrieval.
       *
       * Called from the DeferredResponseProcessor when processing deferred operations.
       *
       * @param response Response returned by the service.
       */
      virtual void ProcessRawResponse(std::unique_ptr<Azure::Core::Http::RawResponse>& response)
          = 0;

      /**
       * @brief Creates a new DeferredResponseSharedBase for the specified request.
       *
       * @param request Request to send to the server eventually.
       */
      explicit DeferredResponseSharedBase(Azure::Core::Http::Request request) : m_request(request)
      {
      }
    };

    /**
     * @brief Shared state for a DeferredResponse.
     *
     * This class implements a deferred operation For deferred operations which have a
     * post-processing callback defined.
     */
    template <typename T> class DeferredResponseShared : public DeferredResponseSharedBase {
    private:
      std::function<Azure::Response<T>(std::unique_ptr<Azure::Core::Http::RawResponse>&)>
          m_completeProcessing;
      std::unique_ptr<Azure::Core::Http::RawResponse> m_rawResponse;

    public:
      explicit DeferredResponseShared(Azure::Core::Http::Request request)
          : DeferredResponseShared(request, nullptr)
      {
      }
      explicit DeferredResponseShared(
          Azure::Core::Http::Request request,
          std::function<Azure::Response<T>(std::unique_ptr<Azure::Core::Http::RawResponse>&)>
              completeProcessing)
          : DeferredResponseSharedBase(request), m_completeProcessing(completeProcessing)
      {
      }
      void ProcessRawResponse(std::unique_ptr<Azure::Core::Http::RawResponse>& response)
      {
        m_rawResponse = std::move(response);
      }

      Response<T> GetResponse() { return m_completeProcessing(m_rawResponse); }
    };

    /**
     * \brief A BatchFactory creates DeferredResponse objects which are created from
     * an HTTP request to be sent to the service.
     *
     * A Batch class can derive from a DeferredResponseFactory to simplify the
     * creation of DeferredResponse<T> objects.
     *
     * @note This implementation of DeferredResponseFactory is derived off of
     * an Azure::Core::Http::Request object. An alternate implementation could take
     * a std::shared_ptr<Xxx> where a specialization of Xxx captures the parameters
     * to the batched operation. This avoids the limitations associated with the BodyStream
     * member of the Result operation (BodyStream references the body data which means
     * that body data needs to be stabilized across batch operations via some other mechanism).
     * 
     * Capturing the parameters to the batched operation also avoids a potential issue
     * associated with the lifetime of authentication tokens - if the lifetime of the
     * authentication token is short (15 minutes or so), it is possible that the token
     * will expire between when the DeferredResponse object is created and when the
     * batched operation is submitted to the server. 
     */
    class DeferredResponseFactory {
      std::vector<std::shared_ptr<DeferredResponseSharedBase>> m_deferredOperations;

    protected:
      virtual std::vector<std::shared_ptr<DeferredResponseSharedBase>> const& DeferredOperations()
      {
        return m_deferredOperations;
      }

    public:
      /***
       * @brief Creates a deferred operation from a customer supplied shared object derived from
       * DeferredResponseSharedBase.
       *
       * This overload allows a customer to derive their own DeferredOperationSharedBase derived
       * class which allows them to process deferred operations without providing a lambda (thus
       * avoiding the challenges associated with lambda captures).
       *
       */
      template <typename T>
      DeferredResponse<T> CreateDeferredResponse(
          std::shared_ptr<DeferredResponseSharedBase> deferredOperationShared)
      {
        m_deferredOperations.push_back(deferredOperationShared);
        return DeferredResponse<T>(deferredOperationShared);
      }

      /***
       * @brief Creates a deferred operation from the supplied HTTP request object.
       *
       * Creates a deferred operation with a callback function which can be used by the caller to
       * process any results returned by the service.
       *
       * NOTE: If the callback function is a C++ lambda, the lambda cannot capture any values by
       * reference - it absolutely will be called in a context outside the function in which the
       * lambda was created.
       *
       */
      template <typename T>
      DeferredResponse<T> CreateDeferredResponse(
          Azure::Core::Http::Request requestToDefer,
          std::function<Azure::Response<T>(std::unique_ptr<Azure::Core::Http::RawResponse>&)>
              completeProcessing)
      {
        std::shared_ptr<DeferredResponseSharedBase> deferredResponse(
            std::make_shared<DeferredResponseShared<T>>(requestToDefer, completeProcessing));
        m_deferredOperations.push_back(deferredResponse);
        return DeferredResponse<T>(deferredResponse);
      }
    };

  } // namespace _internal

  template <typename T> class DeferredResponse {
  private:
    std::shared_ptr<_internal::DeferredResponseSharedBase> m_sharedState;

    friend class _internal::DeferredResponseFactory;
    DeferredResponse(std::shared_ptr<_internal::DeferredResponseSharedBase> sharedState)
        : m_sharedState(sharedState)
    {
    }

  public:
    Response<T> GetResponse() const
    {
      auto sharedState = static_cast<_internal::DeferredResponseShared<T>*>(m_sharedState.get());
      return sharedState->GetResponse();
    }
  };
}} // namespace Azure::Core
