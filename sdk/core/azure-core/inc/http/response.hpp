// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <map>
#include <string>

#include <internal/contract.hpp>
#include "http/bodybuffer.hpp"
#include "http/bodystream.hpp"

namespace Azure { namespace Core { namespace Http {

  class Response
  {

  private:
    Response(Response const&) = delete;
    void operator=(Response const&) = delete;

    uint16_t _statusCode;
    std::string _reasonPhrase;
    std::map<std::string, std::string> _headers;

    // Response can contain no body, or either of next bodies (_bodyBuffer plus size or bodyStream)
    BodyBuffer& _bodyBuffer;
    BodyStream& _bodyStream;

    Response(
        uint16_t statusCode,
        std::string reasonPhrase,
        BodyBuffer& const bodyBuffer,
        BodyStream& const BodyStream)
        : _statusCode(statusCode), _reasonPhrase(reasonPhrase), _bodyBuffer(bodyBuffer),
          _bodyStream(BodyStream)
    {}

  public:
    Response(uint16_t statusCode, std::string reasonPhrase)
        : Response(statusCode, reasonPhrase, *BodyBuffer::null, *BodyStream::null)
    {}

    // Methods used to build HTTP response
    void addHeader(std::string const& name, std::string const& value);
    void setBody(BodyBuffer& const bodyBuffer);
    void setBody(BodyStream& const bodyStream);

    // Methods used by transport layer (and logger) to send response
    uint16_t const& getStatusCode();
    std::string const& getReasonPhrase();
    std::map<std::string, std::string> const& getHeaders();
    BodyStream& getBodyStream();
    BodyBuffer& getBodyBuffer();
  };

}}} // namespace Azure::Core::Http
