// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "policy.hpp"
#include <vector>

namespace Azure { namespace Core { namespace Http {
  class HttpTransport
  {};

  class HttpPipeline
  {
  private:
    HttpTransport _transport;
    vector<unique_ptr<HttpPolicy>> _policies;

  public:
    HttpPipeline(HttpTransport transport, HttpPolicy policies[] = null)
    {
      _transport = transport;
      _policies = policies;
    }

  public:
    virtual int Process(http_message message, HttpPolicy polices[]) = 0;
  };

}}} // namespace Azure::Core::Http
