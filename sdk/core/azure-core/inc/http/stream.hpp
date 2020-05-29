// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

namespace Azure { namespace Core { namespace Http {

  // BodyStream is used to read data to/from a service
  class BodyStream {
  public:
    static BodyStream* null;

    // Returns the length of the data; used with the HTTP Content-Length header
    virtual uint64_t Length() = 0;

    // Resets the stream back to the beginning (for retries)
    // Derived classes that send data in an HTTP request MUST override this and implement it
    // properly.
    virtual void Rewind()
    {
      throw "Not Implemented"; // TODO: Replace with best practice as defined by guideline
    };

    // Reads more data; throws if error/canceled
    // return copied size
    virtual uint64_t Read(/*Context& context, */ uint8_t* buffer, uint64_t count) = 0;

    // Closes the stream; typically called after all data read or if an error occurs.
    virtual void Close() = 0;
  };

  class MemoryBodyStream : public BodyStream {
  private:
    uint8_t* m_ptr;
    uint8_t* m_ptr_possition;
    uint64_t m_length;

  public:
    MemoryBodyStream(uint8_t* ptr, uint64_t length)
        : m_ptr(ptr), m_ptr_possition(ptr), m_length(length)
    {
    }

    // Override methods

    // close does nothing opp
  };

}}} // namespace Azure::Core::Http
