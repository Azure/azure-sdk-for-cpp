// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

namespace Azure { namespace Core { namespace Http {

  class HttpPolicy
  {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual Response Process(Context context, Request request) = 0;

  protected:
    ~HttpPolicy(){}

  }}} // namespace Azure::Core::Http
