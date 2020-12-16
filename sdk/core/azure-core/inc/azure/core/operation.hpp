// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/operation_status.hpp"
#include "azure/core/response.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core {

  //@param <T> The type of the final result of the long-running operation.
  template <class T> class Operation {
  private:
    virtual std::unique_ptr<Http::RawResponse> PollInternal(Context& context) = 0;
    virtual Response<T> PollUntilDoneInternal(std::chrono::milliseconds period, Context& context)
        = 0;

  protected:
    OperationStatus m_status = OperationStatus::NotStarted;

  public:
    OperationStatus Status() const { return m_status; }

    bool IsDone() const
    {
      return (
          m_status == OperationStatus::Succeeded || m_status == OperationStatus::Cancelled
          || m_status == OperationStatus::Failed);
    }

    bool HasValue() const { return (m_status == OperationStatus::Succeeded); }

    virtual T Value() const = 0;

    virtual std::string GetResumeToken() const = 0;

    std::unique_ptr<Http::RawResponse> Poll()
    {
      Context context;
      return PollInternal(context);
    }

    std::unique_ptr<Http::RawResponse> Poll(Context& context) { return PollInternal(context); }

    Response<T> PollUntilDone(std::chrono::milliseconds period)
    {
      Context context;
      return PollUntilDoneInternal(period, context);
    }

    Response<T> PollUntilDone(std::chrono::milliseconds period, Context& context)
    {
      return PollUntilDoneInternal(period, context);
    }
  };
}} // namespace Azure::Core
