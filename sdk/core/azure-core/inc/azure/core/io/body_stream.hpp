// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief BodyStream is used to read data to/from a service.
 */

#pragma once

#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <unistd.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "azure/core/context.hpp"
#include "azure/core/internal/io/file_handle_holder.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace Azure { namespace IO {

  /**
   *@brief Used to read data to/from a service.
   */
  class BodyStream {
  private:
    /**
     * @brief Read portion of data into a buffer.
     *
     * @remark This is the `OnRead` implementation that all derived classes need to provide.
     *
     * @param buffer Pointer to a byte buffer to read the data into.
     * @param count Size of the buffer to read the data into.
     * @param context #Azure::Core::Context so that operation can be cancelled.
     *
     * @return Number of bytes read.
     */
    virtual int64_t OnRead(uint8_t* buffer, int64_t count, Azure::Core::Context const& context) = 0;

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
     * @remark Derived classes that send data in an HTTP request MUST override this and implement
     * it properly.
     */
    virtual void Rewind()
    {
      throw std::logic_error(
          "The specified BodyStream doesn't support Rewind which is required to guarantee fault "
          "tolerance when retrying any operation. Consider creating a MemoryBodyStream or "
          "FileBodyStream, which are rewindable.");
    };

    /**
     * @brief Read portion of data into a buffer.
     * @remark Throws if error/cancelled.
     *
     * @param buffer Pointer to a first byte of the byte buffer to read the data into.
     * @param count Size of the buffer to read the data into.
     * @param context #Azure::Core::Context so that operation can be cancelled.
     *
     * @return Number of bytes read.
     */
    int64_t Read(uint8_t* buffer, int64_t count, Azure::Core::Context const& context)
    {
      context.ThrowIfCancelled();
      return OnRead(buffer, count, context);
    };

    /**
     * @brief Read #Azure::IO::BodyStream into a buffer until the buffer is filled, or until
     * the stream is read to end.
     *
     * @param body #Azure::IO::BodyStream to read.
     * @param buffer Pointer to a first byte of the byte buffer to read the data into.
     * @param count Size of the buffer to read the data into.
     * @param context #Azure::Core::Context so that operation can be cancelled.
     *
     * @return Number of bytes read.
     */
    static int64_t ReadToCount(
        BodyStream& body,
        uint8_t* buffer,
        int64_t count,
        Azure::Core::Context const& context);

    /**
     * @brief Read #Azure::IO::BodyStream until the stream is read to end, allocating memory
     * for the entirety of contents.
     *
     * @param context #Azure::Core::Context so that operation can be cancelled.
     * @param body #Azure::IO::BodyStream to read.
     *
     * @return A vector of bytes containing the entirety of data read from the \p body.
     */
    static std::vector<uint8_t> ReadToEnd(BodyStream& body, Azure::Core::Context const& context);
  };

  /**
   * @brief #Azure::IO::BodyStream providing data from an initialized memory buffer.
   */
  class MemoryBodyStream : public BodyStream {
  private:
    const uint8_t* m_data;
    int64_t m_length;
    int64_t m_offset = 0;

    int64_t OnRead(uint8_t* buffer, int64_t count, Azure::Core::Context const& context) override;

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
     * @param data Pointer to a first byte of the buffer with the contents to provide the data
     * from to the readers.
     * @param length Size of the buffer.
     */
    explicit MemoryBodyStream(const uint8_t* data, int64_t length) : m_data(data), m_length(length)
    {
    }

    int64_t Length() const override { return this->m_length; }

    void Rewind() override { m_offset = 0; }
  };

  /**
   * @brief A concrete implementation of #Azure::IO::BodyStream used for reading data from a file.
   */
  class FileBodyStream : public BodyStream {
  private:
    // immutable
    Azure::IO::_internal::FileHandleHolder m_fileStreamHolder;
#if defined(AZ_PLATFORM_WINDOWS)
    HANDLE m_filehandle;
#elif defined(AZ_PLATFORM_POSIX)
    int m_fileDescriptor;
#endif
    int64_t m_length;
    // mutable
    int64_t m_offset;
    BodyStream* m_parallelBodyStream;

    int64_t OnRead(uint8_t* buffer, int64_t count, Azure::Core::Context const& context) override;

  public:
    /**
     * @brief Construct from a file name.
     *
     * @param filename A reference to a file name string used to identify the file, which needs to
     * have the necessary file path specified to locate the file.
     *
     * @remark The #Azure::IO::FileBodyStream owns the file object and is responsible for opening
     * and closing the file.
     *
     * @remark Do not use the file object to read or write, while it is being used by the stream.
     */
    FileBodyStream(const std::string& filename);

    /**
     * @brief Closes the file and cleans up any resources.
     *
     */
    ~FileBodyStream();

    // Rewind seeks back to 0
    void Rewind() override { this->m_offset = 0; }

    int64_t Length() const override { return this->m_length; };
  };

}} // namespace Azure::IO
