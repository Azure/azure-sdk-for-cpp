// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#ifdef POSIX
#include <unistd.h>
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // Windows

#include <algorithm>
#include <context.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  // BodyStream is used to read data to/from a service
  class BodyStream {
  public:
    virtual ~BodyStream() = 0;

    // Returns the length of the data; used with the HTTP Content-Length header
    virtual int64_t Length() const = 0;

    // Resets the stream back to the beginning (for retries)
    // Derived classes that send data in an HTTP request MUST override this and implement it
    // properly.
    virtual void Rewind()
    {
      throw "Not Implemented"; // TODO: Replace with best practice as defined by guideline
    };

    // Reads more data; throws if error/canceled
    // return copied size
    virtual int64_t Read(Context& context, uint8_t* buffer, int64_t count) = 0;

    // Closes the stream; typically called after all data read or if an error occurs.
    virtual void Close() = 0;

    // Keep reading until buffer is all fill out of the end of stream content is reached
    static int64_t ReadToCount(Context& context, BodyStream& body, uint8_t* buffer, int64_t count);

    static std::unique_ptr<std::vector<uint8_t>> ReadToEnd(Context& context, BodyStream& body);
  };

  class MemoryBodyStream : public BodyStream {
  private:
    const uint8_t* m_data;
    int64_t m_length;
    int64_t m_offset = 0;

  public:
    // Forbid constructor for rval so we don't end up storing dangling ptr
    MemoryBodyStream(std::vector<uint8_t> const&&) = delete;

    MemoryBodyStream(std::vector<uint8_t> const& buffer)
        : MemoryBodyStream(buffer.data(), static_cast<int64_t>(buffer.size()))
    {
    }

    // cast as vector from ptr and length
    explicit MemoryBodyStream(const uint8_t* data, int64_t length) : m_data(data), m_length(length)
    {
    }

    int64_t Length() const override { return this->m_length; }

    int64_t Read(Context& context, uint8_t* buffer, int64_t count) override;

    void Rewind() override { m_offset = 0; }

    void Close() override {}
  };

  // Use for request with no body
  class NullBodyStream : public Azure::Core::Http::BodyStream {
  public:
    explicit NullBodyStream() {}

    ~NullBodyStream() override {}

    int64_t Length() const override { return 0; }

    void Rewind() override {}

    int64_t Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count) override
    {
      (void)context;
      (void)buffer;
      (void)count;
      return 0;
    };

    void Close() override {}
  };

#ifdef POSIX
  class FileBodyStream : public BodyStream {
  private:
    // in mutable
    int m_fd;
    int64_t m_baseOffset;
    int64_t m_length;
    // mutable
    int64_t m_offset;

  public:
    FileBodyStream(int fd, int64_t offset, int64_t length)
        : m_fd(fd), m_baseOffset(offset), m_length(length), m_offset(0)
    {
    }

    ~FileBodyStream() override {}

    // Rewind seek back to 0
    void Rewind() override { this->m_offset = 0; }

    int64_t Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count) override;

    int64_t Length() const override { return this->m_length; };

    // close does nothing opp
    void Close() override {}
  };
#endif

#ifdef WINDOWS
  class FileBodyStream : public BodyStream {
  private:
    // in mutable
    HANDLE m_hFile;
    int64_t m_baseOffset;
    int64_t m_length;
    // mutable
    int64_t m_offset;

  public:
    FileBodyStream(HANDLE hFile, int64_t offset, int64_t length)
        : m_hFile(hFile), m_baseOffset(offset), m_length(length), m_offset(0)
    {
    }

    ~FileBodyStream() override {}

    // Rewind seek back to 0
    void Rewind() override { this->m_offset = 0; }

    int64_t Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count) override;

    int64_t Length() const override { return this->m_length; };

    // close does nothing opp
    void Close() override {}
  };
#endif // Windows

  class LimitBodyStream : public BodyStream {
    BodyStream* m_inner;
    int64_t m_length;
    int64_t m_bytesRead = 0;

    LimitBodyStream(BodyStream* inner, int64_t max_length)
        : m_inner(inner), m_length(std::min(inner->Length(), max_length))
    {
    }

    int64_t Length() const override { return this->m_length; }
    void Rewind() override
    {
      this->m_inner->Rewind();
      this->m_bytesRead = 0;
    }
    int64_t Read(Context& context, uint8_t* buffer, int64_t count) override;

    void Close() override { this->m_inner->Close(); }
  };

}}} // namespace Azure::Core::Http
