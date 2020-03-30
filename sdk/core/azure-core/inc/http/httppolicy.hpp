// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Azure { namespace Core { namespace Http {

  class HttpPolicy
  {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual http_response Process(Context context, Request message, HttpPolicy** policies) = 0;

  protected:
    /* Get Next Policy*/

    ///
    Response NextPolicy(Context context, Request message, HttpPolicy** policies)
    {
      // Not right
      return policies[1]->Process(context, message, ++policies);
    }
  };

}}} // namespace Azure::Core::Http
