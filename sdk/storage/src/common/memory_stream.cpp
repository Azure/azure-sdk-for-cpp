// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/memory_stream.hpp"

#include <algorithm>

namespace Azure { namespace Storage {

  uint64_t MemoryStream::Read(uint8_t* buffer, uint64_t count)
  {
    std::size_t readSize = static_cast<std::size_t>(std::min(count, static_cast<uint64_t>(m_length - m_offset)));
    std::copy(m_data + m_offset, m_data + m_offset + readSize, buffer);
    m_offset += readSize;
    return readSize;
  }

}} // namespace Azure::Storage