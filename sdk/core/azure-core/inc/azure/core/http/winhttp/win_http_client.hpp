#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"

#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class WinHttpTansport : public HttpTransport {
  private:
  public:
    WinHttpTansport();
    ~WinHttpTansport() override;

    virtual std::unique_ptr<RawResponse> Send(Context const& context, Request& request) override;
  };

}}} // namespace Azure::Core::Http
