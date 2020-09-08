// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief BodyStream is used to read data to/from a service.
 */

#pragma once

#ifdef POSIX
#include <unistd.h>
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif // Windows

#include <azure/core/context.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  /**
   *@brief Used to read data to/from a service.
   */
  class BodyStream {
  public:
    /// Destructor.
    virtual ~BodyStream() = default;

    /**
     * @brief Get the length of the data.
     * @remark Used with the HTTP `Content-Length` header.
     */
    virtual int64_t Length() const = 0;

    /*
     * @brief Resets the stream back to the beginning (for retries).
     * @remark Derived classes that send data in an HTTP request MUST override this and implement it
     * properly.
     */
    virtual void Rewind()
    {
      throw "Not Implemented"; // TODO: Replace with best practice as defined by guideline
    };

    /**
     * @brief Read portion of data into a buffer.
     * @remark Throws if error/canceled.
     *
     * @param conntext #Context so that operation can be canceled.
     * @param buffer Pointer to a first byte of the byte buffer to read the data into.
     * @param count Size of the buffer to read the data into.
     *
     * @return Number of bytes read.
     */
    virtual int64_t Read(Context const& context, uint8_t* buffer, int64_t count) = 0;

    /**
     * @brief Read #BodyStream into a buffer until the buffer is filled, or until the stream is read
     * to end.
     *
     * @param conntext #Context so that operation can be canceled.
     * @param body #BodyStream to read.
     * @param buffer Pointer to a first byte of the byte buffer to read the data into.
     * @param count Size of the buffer to read the data into.
     *
     * @return Number of bytes read.
     */
    static int64_t ReadToCount(
        Context const& context,
        BodyStream& body,
        uint8_t* buffer,
        int64_t count);

    /**
     * @brief Read #BodyStream until the stream is read to end, allocating memory for the entirety
     * of contents.
     *
     * @param conntext #Context so that operation can be canceled.
     * @param body #BodyStream to read.
     *
     * @return A vector of bytes containing the entirety of data read from the \p body.
     */
    static std::vector<uint8_t> ReadToEnd(Context const& context, BodyStream& body);
  };

  /**
   * @brief #BodyStream providing data from an initialized memory buffer.
   */
  class MemoryBodyStream : public BodyStream {
  private:
    const uint8_t* m_data;
    int64_t m_length;
    int64_t m_offset = 0;

  public:
    // Forbid constructor for rval so we don't end up storing dangling ptr
    MemoryBodyStream(std::vector<uint8_t> const&&) = delete;

    /**
     * @brief Construct using vector of bytes.
     *
     * @param buffer Vector of bytes with the contents to provide the data from to the readers.
     */
    MemoryBodyStream(std::vector<uint8_t> const& buffer)
        : MemoryBodyStream(buffer.data(), static_cast<int64_t>(buffer.size()))
    {
    }

    /**
     * @brief Construct using buffer pointer and its size.
     *
     * @param data Pointer to a first byte of the buffer with the contents to provide the data from
     * to the readers.
     * @param length Size of the buffer.
     */
    explicit MemoryBodyStream(const uint8_t* data, int64_t length) : m_data(data), m_length(length)
    {
    }

    int64_t Length() const override { return this->m_length; }

    int64_t Read(Context const& context, uint8_t* buffer, int64_t count) override;

    void Rewind() override { m_offset = 0; }
  };

  /**
   * @brief Empty #BodyStream.
   * @remark Used for requests with no body.
   */
  class NullBodyStream : public Azure::Core::Http::BodyStream {
  public:
    /// Constructor.
    explicit NullBodyStream() {}

    int64_t Length() const override { return 0; }

    void Rewind() override {}

    int64_t Read(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override
    {
      (void)context;
      (void)buffer;
      (void)count;
      return 0;
    };

    /**
     * @brief Gets a singleton instance of a #NullBodyStream.
     */
    static NullBodyStream* GetNullBodyStream()
    {
      static NullBodyStream nullBodyStream;
      return &nullBodyStream;
    }
  };

#ifdef POSIX
  /**
   * @brief #BodyStream providing its data from a file.
   */
  class FileBodyStream : public BodyStream {
  private:
    // in mutable
    int m_fd;
    int64_t m_baseOffset;
    int64_t m_length;
    // mutable
    int64_t m_offset;

  public:
    /**
     * @brief Construct from a file.
     *
     * @param fd File descriptor.
     * @param offset Offset in the file to start providing the data from.
     * @param length Length of the data, in bytes, to provide.
     */
    FileBodyStream(int fd, int64_t offset, int64_t length)
        : m_fd(fd), m_baseOffset(offset), m_length(length), m_offset(0)
    {
    }

    // Rewind seek back to 0
    void Rewind() override { this->m_offset = 0; }

    int64_t Read(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override;

    int64_t Length() const override { return this->m_length; };
  };
#endif

#ifdef WINDOWS
  /**
   * @brief #BodyStream providing its data from a file.
   */
  class FileBodyStream : public BodyStream {
  private:
    // in mutable
    HANDLE m_hFile;
    int64_t m_baseOffset;
    int64_t m_length;
    // mutable
    int64_t m_offset;

  public:
    /**
     * @brief Construct from a file.
     *
     * @param hFile File handle.
     * @param offset Offset in the file to start providing the data from.
     * @param length Length of the data, in bytes, to provide.
     */
    FileBodyStream(HANDLE hFile, int64_t offset, int64_t length)
        : m_hFile(hFile), m_baseOffset(offset), m_length(length), m_offset(0)
    {
    }

    // Rewind seek back to 0
    void Rewind() override { this->m_offset = 0; }

    int64_t Read(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override;

    int64_t Length() const override { return this->m_length; };
  };
#endif // Windows

  /**
   * @brief #BodyStream that provides its data from another #BodyStream.
   */
  class LimitBodyStream : public BodyStream {
  private:
    BodyStream* m_inner;
    int64_t m_length;
    int64_t m_bytesRead = 0;

  public:
    /**
     * @brief Construct from another #BodyStream.
     *
     * @param inner #BodyStream to provide the data from to the readers.
     * @param max_length Maximum number of bytes to provide to the readers.
     */
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
    int64_t Read(Context const& context, uint8_t* buffer, int64_t count) override;
  };

}}} // namespace Azure::Core::Http
