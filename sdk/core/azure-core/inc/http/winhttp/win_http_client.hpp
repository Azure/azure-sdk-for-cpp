#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/http.hpp"
#include "http/http_client.hpp"
#include "http/policy.hpp"

#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class WinHttpTansport : public Transport {
  private:

  public:
    WinHttpTansport();
    ~WinHttpTansport();

    virtual Response Send(Context& context, Request& request) ;
  };

}}} // namespace Azure::Core::Http
