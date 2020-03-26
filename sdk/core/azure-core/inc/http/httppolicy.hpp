// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"

namespace azure { namespace core { namespace http {
  // TODO - Temporary definition until message is added
  class HttpRequest
  {};

  // TODO - Temporary definition until message is added
  class http_response
  {};

  class HttpPolicy
  {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual http_response process(Context context, HttpRequest message, HttpPolicy** policies) = 0;

  protected:
    /* Get Next Policy*/

    ///
    http_response next_policy(Context context, HttpRequest message, HttpPolicy** policies)
    {
      // Not right
      return policies[1]->process(context, message, ++policies);
    }

  };

}}} // namespace azure::core::http
