// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/operation_status.hpp"
#include "azure/core/response.hpp"

#include <chrono>

namespace Azure { namespace Core {

  //@param <T> The type of the final result of the long-running operation.
  template <class T> class Operation {
  protected:
    OperationStatus m_status = OperationStatus::NotStarted;
  public:
    OperationStatus Status() { return m_status; }

    bool Done() {
      return (
          m_status == OperationStatus::Succeeded ||
          m_status == OperationStatus::Cancelled ||
          m_status == OperationStatus::Failed);
    }

    bool HasValue()
    {
      return (m_status == OperationStatus::Succeeded);
    }

    virtual T Value() = 0;

    virtual std::string GetResumeToken() = 0;

    virtual std::unique_ptr<Http::RawResponse> Poll(Context& context = Context()) = 0;

    virtual Response<T> PollUntilDone(std::chrono::milliseconds period, Context& context = Context()) = 0;

  };
}} // namespace Azure::Core
