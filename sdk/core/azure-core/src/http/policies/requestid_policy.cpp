// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/httppolicy.hpp"

#include <string>

namespace azure { namespace core { namespace http { namespace foo {
  struct RequestIdPolicyOptions
  {
  public:
    RequestIdPolicyOptions(){
        // Set some values
    };
  };

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

    http_response Process(Context ctx, HttpRequest message, HttpPolicy** policies) override
    {
      (void*)policies;
      (void)message;

      // Do real work here

      return NextPolicy(ctx, message, policies);
    }
  };

  const std::string RequestIdPolicy::X_MS_REQUEST_ID = "x-ms-request-id";
}}}} // namespace azure::core::http::foo