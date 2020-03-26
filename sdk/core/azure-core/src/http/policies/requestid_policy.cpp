// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/httppolicy.hpp"

#include <string>

namespace azure {
namespace core {
  namespace http {
    namespace foo {
      struct requestid_policy_options
      {
      public:
        requestid_policy_options(){
            // Set some values
        };
      };

      class requestid_policy : HttpPolicy
      {
      private:
        //
        static const std::string X_MS_REQUEST_ID;
        requestid_policy_options m_requestid_policy_options;

      public:
        requestid_policy(requestid_policy_options options)
        {
          // Ensure this is a copy
          //  Assert
          m_requestid_policy_options = options;
        }

        http_response process(Context ctx, HttpRequest message, HttpPolicy** policies) override
        {
          (void*)policies;
          (void)message;

          // Do real work here

          return next_policy(ctx, message, policies);
        }
      };

      const std::string requestid_policy::X_MS_REQUEST_ID = "x-ms-request-id";
    } // namespace foo
  } // namespace http
} // namespace core
} // namespace azure