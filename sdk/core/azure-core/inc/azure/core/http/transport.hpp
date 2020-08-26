// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"

namespace Azure { namespace Core { namespace Http {

  class HttpTransport {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions

    // TODO - Should this be const
    virtual std::unique_ptr<RawResponse> Send(Context const& context, Request& request) = 0;
    virtual ~HttpTransport() {}

  protected:
    HttpTransport() = default;
    HttpTransport(const HttpTransport& other) = default;
    HttpTransport(HttpTransport&& other) = default;
    HttpTransport& operator=(const HttpTransport& other) = default;
  };

}}} // namespace Azure::Core::Http
