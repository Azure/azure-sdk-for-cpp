// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/long_running_operation_state.hpp"
#include "azure/core/response.hpp"

#include <chrono>

/*
Need two things:
  Polling loop that does the wait for the customer
  Enabling the scenario where the user can just wait
*/

namespace Azure { namespace Core {
  /*
  template <class T> class OperationResponse {
  public:
    LongRunningOperationState m_status;
    Nullable<T> m_value;
    int m_retryAfter;  //Duration to wait before next retry

  public:
    OperationResponse(LongRunningOperationState status, T value, int retryAfter)
        : m_status(status), m_value(value), m_retryAfter(retryAfter){};

    T GetValue() { return m_value; }

    int GetRetryDuration() { return m_retryAfter; }

  };
  */

  //@param <T> The type of the final result of the long-running operation.
  template <class T> class Operation {
  public:
    virtual bool Done() = 0;

    virtual std::string GetResumeToken() = 0;

    virtual std::unique_ptr<Http::RawResponse> Poll(Context& context) = 0;

    virtual Response<T> PollUntilDone(Context& context, std::chrono::milliseconds period) = 0;
  };
}} // namespace Azure::Core
