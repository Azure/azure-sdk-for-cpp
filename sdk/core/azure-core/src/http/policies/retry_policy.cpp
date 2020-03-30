// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http_policy.hpp"

#include <string>

namespace azure
{
namespace core
{
  namespace http
  {
    struct retry_policy_options
    {

    private:
      int16_t max_retries;
      int32_t retry_delay_msec;

    public:
      retry_policy_options(){ 
        max_retries = 5;
        retry_delay_msec = 500;
      };
    };

    class retry_policy : HttpPolicy
    {
    private:
      retry_policy_options m_retry_policy_options;

    public:
      retry_policy(retry_policy_options options)
      {
        // Ensure this is a copy
        //  Assert
        requestIdPolicyOptions = options;
      }

      http_response Process(HttpRequest message, HttpPolicy** policies) override
      {
        (void*)policies;
        (void)message;

        // Do real work here

        return NextPolicy(message, policies);
      }
    };

  } // namespace http
} // namespace core
} // namespace azure