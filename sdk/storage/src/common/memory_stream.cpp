// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/memory_stream.hpp"

#include <algorithm>

namespace Azure { namespace Storage {

  int64_t MemoryStream::Read(uint8_t* buffer, int64_t count)
  {
    int64_t readSize
        = static_cast<int64_t>(std::min(count, static_cast<int64_t>(m_length - m_offset)));
    std::copy(m_data + m_offset, m_data + m_offset + readSize, buffer);
    m_offset += readSize;
    return readSize;
  }

}} // namespace Azure::Storage
