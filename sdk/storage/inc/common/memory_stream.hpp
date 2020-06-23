// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/stream.hpp"

#include <vector>

namespace Azure { namespace Storage {

  class MemoryStream : public Azure::Core::Http::BodyStream {
  public:
    explicit MemoryStream(const char* data, std::size_t length) : m_data(data), m_length(length) {}

    explicit MemoryStream(const std::vector<uint8_t>& data)
        : MemoryStream(reinterpret_cast<const char*>(data.data()), data.size())
    {
    }

    ~MemoryStream() override {}

    uint64_t Length() const override { return m_length; }

    void Rewind() override { m_offset = 0; }

    uint64_t Read(uint8_t* buffer, uint64_t count) override;

    void Close() override {}

  private:
    const char* m_data;
    std::size_t m_length;
    std::size_t m_offset = 0;
  };

}} // namespace Azure::Storage