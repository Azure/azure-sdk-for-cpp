// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <map>
#include <string>

#include <internal/contract.hpp>

namespace azure
{
namespace core
{
namespace http
{

// BodyStream is used to read data to/from a service
class BodyStream
{
public:
  static BodyStream* null;

  // Returns the length of the data; used with the HTTP Content-Length header
  virtual uint64_t Length() = 0;

  // Resets the stream back to the beginning (for retries)
  // Derived classes that send data in an HTTP request MUST override this and implement it properly.
  virtual void Rewind()
  {
    throw "Not Implemented"; // TODO: Replace with best practice as defined by guideline
  };

  // Reads more data; EOF if return < count; throws if error/canceled
  virtual uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t offset, uint64_t count)
      = 0;

  // Closes the stream; typically called after all data read or if an error occurs.
  virtual void Close() = 0;
};

class BodyBuffer
{
public:
  static BodyBuffer* null;

  uint8_t const* _bodyBuffer;
  uint64_t _bodyBufferSize;
  BodyBuffer(uint8_t const* bodyBuffer, uint64_t bodyBufferSize)
      : _bodyBuffer(bodyBuffer), _bodyBufferSize(bodyBufferSize)
  {
  }
};

enum class HttpMethod
{
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  PATCH,
};

} // namespace http
} // namespace core
} // namespace azure
