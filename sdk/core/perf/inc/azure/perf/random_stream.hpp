// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A random stream of any specific size. Useful for test cases.
 *
 */

#pragma once

#include <azure/core/io/body_stream.hpp>

#include <memory>
#include <random>

namespace Azure { namespace Perf {

  /**
   * @brief A random stream of any specific size. Useful for test cases.
   *
   */
  class RandomStream {
  private:
    /**
     * @brief Wraps a stream and keep reading bytes from it by rewinding it until some length.
     *
     * @note Enables to create a stream with huge size by re-using a small buffer.
     *
     */
    class CircularStream : public Azure::Core::IO::BodyStream {
    private:
      std::unique_ptr<std::vector<uint8_t>> m_buffer;
      size_t m_length;
      size_t m_totalRead = 0;
      Azure::Core::IO::MemoryBodyStream m_memoryStream;

      size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override;

    public:
      CircularStream(size_t size);

      int64_t Length() const override { return this->m_length; }
      void Rewind() override { m_totalRead = 0; }
    };

  public:
    static std::unique_ptr<Azure::Core::IO::BodyStream> Create(size_t size)
    {
      return std::make_unique<CircularStream>(size);
    }
  };
}} // namespace Azure::Perf