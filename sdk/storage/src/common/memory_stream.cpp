// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/memory_stream.hpp"

#include <algorithm>
#include <context.hpp>

namespace Azure { namespace Storage {

  int64_t MemoryStream::Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count)
  {
    context.ThrowIfCanceled();

    int64_t readSize
        = static_cast<int64_t>(std::min(count, static_cast<int64_t>(m_length - m_offset)));
    std::copy(m_data + m_offset, m_data + m_offset + readSize, buffer);
    m_offset += readSize;
    return readSize;
  }

}} // namespace Azure::Storage
