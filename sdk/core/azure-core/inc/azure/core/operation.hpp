// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides support for long-running operations.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/operation_status.hpp"
#include "azure/core/response.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core {

  /**
   * @brief Methods starting long-running operations return Operation<T> types.
   *
   * @tparam T The long-running operation final result type.
   */
  template <class T> class Operation {
  private:
    // These are pure virtual b/c the derived class must provide an implementation
    virtual std::unique_ptr<Http::RawResponse> PollInternal(Context& context) = 0;
    virtual Response<T> PollUntilDoneInternal(std::chrono::milliseconds period, Context& context)
        = 0;

  protected:
    std::unique_ptr<Azure::Core::Http::RawResponse> m_rawResponse = nullptr;
    OperationStatus m_status = OperationStatus::NotStarted;

    Operation() = default;

    // Define how an Operation<T> can be moved-constructed from rvalue other. Parameter `other`
    // gave up ownership for the rawResponse.
    Operation(Operation&& other)
        : m_rawResponse(std::move(other.m_rawResponse)), m_status(other.m_status)
    {
    }

    // Define how an Operation<T> can be copy-constructed from some other Operation reference.
    // Operation will create a clone of the rawResponse from `other`.
    Operation(Operation const& other)
        : m_rawResponse(std::make_unique<Http::RawResponse>(other.GetRawResponse())),
          m_status(other.m_status)
    {
    }

    // Define how an Operation<T> can be moved-assigned from rvalue other. Parameter `other`
    // gave up ownership for the rawResponse.
    void operator=(Operation&& other)
    {
      this->m_rawResponse = std::move(other.m_rawResponse);
      this->m_status = other.m_status;
    }

    // Define how an Operation<T> can be copy-assigned from some other Operation reference.
    // Operation will create a clone of the rawResponse from `other`.
    void operator=(Operation const& other)
    {
      this->m_rawResponse = std::make_unique<Http::RawResponse>(other.GetRawResponse());
      this->m_status = other.m_status;
    }

  public:
    virtual ~Operation() {}

    /**
     * @brief Final result of the long-running operation.
     *
     * @return Response<T> the final result of the long-running operation.
     */
    virtual T Value() const = 0;

    /**
     * @brief Gets an token representing the operation that can be used to poll for the status of
     * the long-running operation.
     *
     * @return std::string The resume token.
     */
    virtual std::string GetResumeToken() const = 0;

    /**
     * @brief Get the raw HTTP response.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    virtual Azure::Core::Http::RawResponse const& GetRawResponse() const = 0;

    /**
     * @brief Returns the current #Azure::Core::OperationStatus of the long-running operation.
     */
    OperationStatus Status() const noexcept { return m_status; }

    /**
     * @brief Returns true if the long-running operation completed.
     *
     * @return `true` if the long-running operation is done. `false` otherwise.
     */
    bool IsDone() const noexcept
    {
      return (
          m_status == OperationStatus::Succeeded || m_status == OperationStatus::Cancelled
          || m_status == OperationStatus::Failed);
    }

    /**
     * @brief Returns true if the long-running operation completed successfully and has produced a
     * final result.  The final result is accessible from Value().
     *
     * @return `true` if the long-running operation completed successfully. `false` otherwise.
     */
    bool HasValue() const noexcept { return (m_status == OperationStatus::Succeeded); }

    /**
     * @brief Calls the server to get updated status of the long-running operation.
     *
     * @return An HTTP #Azure::Core::Http::RawResponse returned from the service.
     */
    Http::RawResponse const& Poll()
    {
      // In the cases where the customer doesn't want to use a context we new one up and pass it
      // through
      return Poll(Context::GetApplicationContext());
    }

    /**
     * @brief Calls the server to get updated status of the long-running operation.
     *
     * @param context #Azure::Core::Context allows the user to cancel the long-running operation.
     *
     * @return An HTTP #Azure::Core::Http::RawResponse returned from the service.
     */
    Http::RawResponse const& Poll(Context& context)
    {
      context.ThrowIfCancelled();
      m_rawResponse = PollInternal(context);
      return *m_rawResponse;
    }

    /**
     * @brief Periodically calls the server till the long-running operation completes.
     *
     * @param period Time in milliseconds to wait between polls.
     *
     * @return Response<T> the final result of the long-running operation.
     */
    Response<T> PollUntilDone(std::chrono::milliseconds period)
    {
      // In the cases where the customer doesn't want to use a context we new one up and pass it
      // through
      return PollUntilDone(period, Context::GetApplicationContext());
    }

    /**
     * @brief Periodically calls the server till the long-running operation completes;
     *
     * @param period Time in milliseconds to wait between polls
     * @param context #Azure::Core::Context allows the user to cancel the long-running operation.
     *
     * @return Response<T> the final result of the long-running operation.
     */
    Response<T> PollUntilDone(std::chrono::milliseconds period, Context& context)
    {
      context.ThrowIfCancelled();
      return PollUntilDoneInternal(period, context);
    }
  };
}} // namespace Azure::Core
