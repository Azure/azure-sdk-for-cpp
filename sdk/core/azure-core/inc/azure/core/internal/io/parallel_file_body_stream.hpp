// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An internal implementation of Azure::IO::BodyStream that supports reading files in
 * parallel.
 *
 */
#pragma once

#include "azure/core/io/body_stream.hpp"

namespace Azure { namespace IO { namespace Internal {

  /**
   * @brief #Azure::IO::BodyStream providing its data from a file used for reading in parallel.
   */
  class ParallelFileBodyStream : public Azure::IO::BodyStream {
  private:
    // immutable
#if defined(AZ_PLATFORM_POSIX)
    int m_fileDescriptor;
#elif defined(AZ_PLATFORM_WINDOWS)
    HANDLE m_filehandle;
#endif
    int64_t m_baseOffset;
    int64_t m_length;
    // mutable
    int64_t m_offset;

    int64_t OnRead(uint8_t* buffer, int64_t count, Azure::Core::Context const& context) override;

  public:
#if defined(AZ_PLATFORM_POSIX)
    /**
     * @brief Construct from a file descriptor.
     *
     * @param fileDescriptor A file descriptor to an already opened file object that can be used to
     * identify the file.
     * @param offset The offset from the beginning of the file from which to start accessing the
     * data.
     * @param length The amounts of bytes, starting from the offset, that this stream can access
     * from the file.
     *
     * @remark The caller owns the file handle and needs to open it along with keeping it alive for
     * the necessary duration. The caller is also responsible for closing it once they are done.
     */
    ParallelFileBodyStream(int fileDescriptor, int64_t offset, int64_t length)
        : m_fileDescriptor(fileDescriptor), m_baseOffset(offset), m_length(length), m_offset(0)
    {
    }
#elif defined(AZ_PLATFORM_WINDOWS)
    /**
     * @brief Construct from a file handle.
     *
     * @param fileHandle A file handle to an already opened file object that can be used to identify
     * the file.
     * @param offset The offset from the beginning of the file from which to start accessing the
     * data.
     * @param length The amounts of bytes, starting from the offset, that this stream can access
     * from the file.
     *
     * @remark The caller owns the file handle and needs to open it along with keeping it alive for
     * the necessary duration. The caller is also responsible for closing it once they are done.
     */
    ParallelFileBodyStream(HANDLE fileHandle, int64_t offset, int64_t length)
        : m_filehandle(fileHandle), m_baseOffset(offset), m_length(length), m_offset(0)
    {
    }
#endif

    // Rewind seeks back to 0
    void Rewind() override { this->m_offset = 0; }

    int64_t Length() const override { return this->m_length; };
  };

}}} // namespace Azure::IO::Internal
