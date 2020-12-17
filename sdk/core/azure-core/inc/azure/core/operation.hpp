// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides support for long-running operations.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/operation_state.hpp"
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

    //These are pure virtual b/c the derived class must provide an implementation
    virtual std::unique_ptr<Http::RawResponse> PollInternal(Context& context) = 0;
    virtual Response<T> PollUntilDoneInternal(Context& context, std::chrono::milliseconds period)
        = 0;

  protected:
    OperationState m_state = OperationState::NotStarted;

  public:
    /**
     * @brief Final reuslt of the long-running operation.
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
     * @brief Returns the current #OperationState of the long-running operation.
     */
    OperationState State() const { return m_state; }

    /**
     * @brief Returns true if the long-running operation completed.
     *
     * @return `true` if the long-running operation is done. `false` otherwise.
     */
    bool IsDone() const
    {
      return (
          m_state == OperationState::Succeeded || m_state == OperationState::Cancelled
          || m_state == OperationState::Failed);
    }

    /**
     * @brief Returns true if the long-running operation completed successfully and has produced a
     * final result.  The final result is accessible from Value().
     *
     * @return `true` if the long-running operation completed successfully. `false` otherwise.
     */
    bool HasValue() const { return (m_state == OperationState::Succeeded); }

    /**
     * @brief Calls the server to get updated status of the long-running operation.
     *
     * @return An HTTP #RawResponse returned from the service.
     */
    std::unique_ptr<Http::RawResponse> Poll()
    {
      // In the cases where the customer doesn't want to use a context we new one up and pass it
      // through
      Context context = Context();
      return PollInternal(context);
    }

    /**
     * @brief Calls the server to get updated status of the long-running operation.
     *
     * @param context #Context allows the user to cancel the long-running operation.
     *
     * @return An HTTP #RawResponse returned from the service.
     */
    std::unique_ptr<Http::RawResponse> Poll(Context& context) { return PollInternal(context); }

    /**
     * @brief Periodically calls the server till the long-running operation completes;
     *
     * @param period Time in milliseconds to wait between polls
     *
     * @return Response<T> the final result of the long-running operation.
     */
    Response<T> PollUntilDone(std::chrono::milliseconds period)
    {
      // In the cases where the customer doesn't want to use a context we new one up and pass it
      // through
      Context context = Context();
      return PollUntilDoneInternal(context, period);
    }

    /**
     * @brief Periodically calls the server till the long-running operation completes;
     *
     * @param context #Context allows the user to cancel the long-running operation.
     * @param period Time in milliseconds to wait between polls
     *
     * @return Response<T> the final result of the long-running operation.
     */
    Response<T> PollUntilDone(Context& context, std::chrono::milliseconds period)
    {
      return PollUntilDoneInternal(context, period);
    }
  };
}} // namespace Azure::Core
