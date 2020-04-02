// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/httppolicy.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

#include <string>

namespace Azure { namespace Core { namespace Http {
  struct RetryPolicyOptions
  {

  private:
    int16_t max_retries;
    int32_t retry_delay_msec;

  public:
    RetryPolicyOptions()
    {
      max_retries = 5;
      retry_delay_msec = 500;
    };
  };

  template <class HttpPolicy>
  class RetryPolicy : HttpPolicy
  {
  private:
    RetryPolicyOptions retryPolicyOptions;

  public:
    RetryPolicy(RetryPolicyOptions options)
    {
      // Ensure this is a copy
      //  Assert
      retryPolicyOptions = options;
    }

    Response Process(Context ctx, HttpRequest message) const override
    {
      (void*)policies;
      (void)message;

      // Do real work here

      return HttpPolicy->Process(ctx, message);
    }
  };

}}} // namespace Azure::Core::Http
