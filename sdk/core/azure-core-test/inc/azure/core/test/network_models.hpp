// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the network models for recording HTTP requests from the network.
 *
 */

#pragma once

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/io/body_stream.hpp>

#include <list>
#include <map>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief The mode how the tests cases wil behave.
   *
   */
  enum class TestMode
  {
    PLAYBACK,
    RECORD,
    LIVE,
  };

  /**
   * @brief Keeps track of network call records from each unit test session.
   *
   */
  struct NetworkCallRecord
  {
    std::string Method;
    std::string Url;
    std::map<std::string, std::string> Headers;
    std::map<std::string, std::string> Response;
  };

  /**
   * @brief Keeps track of the network calls and variable names that were made in a test
   * session.
   *
   */
  class RecordedData {
  public:
    std::list<NetworkCallRecord> NetworkCallRecords;
    std::list<std::string> Variables;
  };

  /**
   * @brief A body stream which holds the memory inside.
   *
   * @remark The playback http uses this body stream to be returned as part of the raw response so
   * the transport policy can read from it.
   *
   */
  class WithMemoryBodyStream : public Azure::Core::IO::BodyStream {
  private:
    std::vector<uint8_t> m_memory;
    Azure::Core::IO::MemoryBodyStream m_streamer;

    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override
    {
      return m_streamer.Read(buffer, count, context);
    }

  public:
    // Forbid constructor for rval so we don't end up storing dangling ptr
    WithMemoryBodyStream(std::vector<uint8_t> const&&) = delete;

    /**
     * @brief Construct using vector of bytes.
     *
     * @param buffer Vector of bytes with the contents to provide the data from to the readers.
     */
    WithMemoryBodyStream(std::vector<uint8_t> const& buffer)
        : m_memory(buffer), m_streamer(m_memory)
    {
    }

    int64_t Length() const override { return m_streamer.Length(); }

    void Rewind() override { m_streamer.Rewind(); }
  };

  /**
   * @brief Wraps a stream and keep reading bytes from it by rewinding it until some length.
   *
   * @note Enables to create a stream with huge size by re-using a small buffer (1Mb).
   *
   */
  class CircularBodyStream : public Azure::Core::IO::BodyStream {
  private:
    std::unique_ptr<std::vector<uint8_t>> m_buffer;
    size_t m_length;
    size_t m_totalRead = 0;
    Azure::Core::IO::MemoryBodyStream m_memoryStream;

    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override
    {
      auto available = m_length - m_totalRead;
      if (available == 0)
      {
        return 0;
      }

      auto toRead = std::min(count, available);
      auto read = m_memoryStream.Read(buffer, toRead, context);

      // Circurlar implementation. Rewind the stream every time we reach the end
      if (read == 0) // No more bytes to read from.
      {
        m_memoryStream.Rewind();
        read = m_memoryStream.Read(buffer, toRead, context);
      }

      m_totalRead += read;
      return read;
    }

  public:
    CircularBodyStream(size_t size, uint8_t fillWith)
        : m_buffer(std::make_unique<std::vector<uint8_t>>(1024 * 1024, fillWith)), m_length(size),
          m_memoryStream(*m_buffer)
    {
    }

    int64_t Length() const override { return m_length; }
    void Rewind() override { m_totalRead = 0; }
  };

}}} // namespace Azure::Core::Test