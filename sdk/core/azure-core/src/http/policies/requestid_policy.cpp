// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/httppolicy.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

#include <string>

namespace Azure { namespace Core { namespace Http {
  struct RequestIdPolicyOptions
  {
  public:
    RequestIdPolicyOptions(){
        // Set some values
    };
  };

  template <class HttpPolicy>
  class RequestIdPolicy : HttpPolicy
  {
  private:
    //
    static const std::string X_MS_REQUEST_ID;
    RequestIdPolicyOptions requestIdPolicyOptions_;

  public:
    RequestIdPolicy(RequestIdPolicyOptions options)
    {
      // Ensure this is a copy
      //  Assert
      requestIdPolicyOptions_ = options;
    }

    Response Process(Context ctx, Request message) override
    {
      (void*)policies;
      (void)message;

      // Do real work here

      return HttpPolicy->Process(ctx, message);
    }
  };

  const std::string RequestIdPolicy::X_MS_REQUEST_ID = "x-ms-request-id";
}}} // namespace Azure::Core::Http
