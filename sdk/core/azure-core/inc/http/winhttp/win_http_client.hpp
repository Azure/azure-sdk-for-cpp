#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/policy.hpp"

#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class WinHttpTansport : public HttpTransport {
  private:
  public:
    WinHttpTansport();
    ~WinHttpTansport();

    virtual std::unique_ptr<RawResponse> Send(Context const& context, Request& request);
  };

}}} // namespace Azure::Core::Http
