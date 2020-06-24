// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  // BodyStream is used to read data to/from a service
  class BodyStream {
  public:
    // Returns the length of the data; used with the HTTP Content-Length header
    virtual uint64_t Length() const = 0;

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
    std::vector<uint8_t> m_buffer;
    uint64_t m_offset = 0;

  public:
    MemoryBodyStream(std::vector<uint8_t> buffer) : m_buffer(std::move(buffer)) {}

    // cast as vector from ptr and length
    MemoryBodyStream(uint8_t* ptr, uint64_t length) : m_buffer(ptr, ptr + length) {}

    uint64_t Length() const override { return this->m_buffer.size(); }

    uint64_t Read(uint8_t* buffer, uint64_t count) override
    {
      uint64_t copy_length = std::min(count, (this->m_buffer.size() - m_offset));
      // Copy what's left or just the count
      std::memcpy(buffer, m_buffer.data() + m_offset, (size_t)copy_length);
      // move position
      m_offset += copy_length;

      return copy_length;
    }

    void Rewind() override { m_offset = 0; }

    void Close() override {}
  };

  /*
  TODO: fix file to work multi-platform
  class FileBodyStream : public BodyStream {
  private:
    FILE* stream;
    uint64_t length;

  public:
    FileBodyStream(FILE* stream)
    {
      // set internal fields
      this->stream = stream;
      // calculate size seeking end...
      this->length = fseeko64(stream, 0, SEEK_END);
      // seek back to beggin
      this->Rewind();
    }

    // Rewind seek back to 0
    void Rewind() { rewind(this->stream); }

    uint64_t Read( uint8_t* buffer, uint64_t count)
    {
      // do static cast here?
      return (uint64_t)fread(buffer, 1, count, this->stream);
    }

    // close does nothing opp
    void Close() { fclose(this->stream); }
  }; */

  class LimitBodyStream : public BodyStream {
    BodyStream* m_inner;
    uint64_t m_length;
    uint64_t m_bytesRead = 0;

    LimitBodyStream(BodyStream* inner, uint64_t max_length)
        : m_inner(inner), m_length(std::min(inner->Length(), max_length))
    {
    }

    uint64_t Length() const override { return this->m_length; }
    void Rewind() override
    {
      this->m_inner->Rewind();
      this->m_bytesRead = 0;
    }
    uint64_t Read(uint8_t* buffer, uint64_t count) override
    {
      // Read up to count or whatever length is remaining; whichever is less
      uint64_t bytesRead
          = m_inner->Read(buffer, std::min(count, this->m_length - this->m_bytesRead));
      this->m_bytesRead += bytesRead;
      return bytesRead;
    }
    void Close() override { this->m_inner->Close(); }
  };

}}} // namespace Azure::Core::Http
