// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/platform.hpp>

#include "azure/perf/random_stream.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#endif

static thread_local std::mt19937_64 random_generator(std::random_device{}());

namespace {

static uint8_t RandomChar()
{
  static const uint8_t charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);
  return charset[distribution(random_generator)];
}

void RandomBuffer(uint8_t* buffer, size_t length)
{
  uint8_t* start_addr = buffer;
  uint8_t* end_addr = buffer + length;

  const size_t rand_int_size = sizeof(uint64_t);
  while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
  {
    *(start_addr++) = RandomChar();
  }

  std::uniform_int_distribution<uint64_t> distribution(0ULL, std::numeric_limits<uint64_t>::max());
  while (start_addr + rand_int_size <= end_addr)
  {
    *reinterpret_cast<uint64_t*>(start_addr) = distribution(random_generator);
    start_addr += rand_int_size;
  }
  while (start_addr < end_addr)
  {
    *(start_addr++) = RandomChar();
  }
}

static constexpr size_t DefaultRandomStreamBufferSize = 1024 * 1024;

} // namespace

Azure::Perf::RandomStream::CircularStream::CircularStream(size_t size)
    : m_buffer(std::make_unique<std::vector<uint8_t>>(DefaultRandomStreamBufferSize)),
      m_length(size), m_memoryStream(*m_buffer)
{
  RandomBuffer(m_buffer->data(), DefaultRandomStreamBufferSize);
}

size_t Azure::Perf::RandomStream::CircularStream::OnRead(
    uint8_t* buffer,
    size_t count,
    Azure::Core::Context const& context)
{
  size_t available = m_length - m_totalRead;
  if (available == 0)
  {
    return 0;
  }

  size_t toRead = std::min(count, available);
  auto read = m_memoryStream.Read(buffer, toRead, context);

  // Circurl implementation. Rewind the stream every time we reach the end
  if (read == 0) // No more bytes to read from.
  {
    m_memoryStream.Rewind();
    read = m_memoryStream.Read(buffer, toRead, context);
  }

  m_totalRead += read;
  return read;
}
