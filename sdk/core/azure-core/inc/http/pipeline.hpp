// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/httppolicy.hpp"
#include "http/request.hpp"
#include "context.hpp"

#include <vector>

namespace Azure { namespace Core { namespace Http {
  class HttpTransport
  {};

  class HttpPipeline
  {
  private:
    HttpTransport _transport;
    //vector<unique_ptr<HttpPolicy>> _policies;

  public:
    HttpPipeline(HttpTransport transport /* Send in the policies */)
    {
      _transport = transport;
      //_policies = policies;
    }

  public:
    virtual int SendRequest(Context ctx, Request message) = 0;
  };

}}} // namespace Azure::Core::Http
