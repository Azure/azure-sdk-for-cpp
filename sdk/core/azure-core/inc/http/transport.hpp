// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "http.hpp"

namespace Azure { namespace Core { namespace Http {

  class Transport {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions

    //TODO - Should this be const
    virtual std::unique_ptr<Response> Send(Context& context, Request& request) = 0;
    virtual ~Transport() {}

  protected:
    Transport() = default;
    Transport(const Transport& other) = default;
    Transport(Transport&& other) = default;
    Transport& operator=(const Transport& other) = default;
  };

}}} // namespace Azure::Core::Http
