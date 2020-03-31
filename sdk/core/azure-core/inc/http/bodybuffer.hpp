// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <map>
#include <string>

#include <http/request.hpp>
#include <http/response.hpp>
#include <internal/contract.hpp>

namespace Azure { namespace Core { namespace Http {

  class BodyBuffer
  {
  public:
    static BodyBuffer* null;

    uint8_t const* _bodyBuffer;
    uint64_t _bodyBufferSize;

  public:
    BodyBuffer(uint8_t const* bodyBuffer, uint64_t bodyBufferSize)
        : _bodyBuffer(bodyBuffer), _bodyBufferSize(bodyBufferSize)
    {}
  };

}}} // namespace Azure::Core::Http
