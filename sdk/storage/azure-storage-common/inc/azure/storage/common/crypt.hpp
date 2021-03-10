// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "azure/core/cryptography/hash.hpp"
#include <azure/core/base64.hpp>

namespace Azure { namespace Storage {

  class Crc64Hash : public Azure::Core::Cryptography::Hash {
  public:
    void Concatenate(const Crc64Hash& other);

    ~Crc64Hash() override = default;

  private:
    uint64_t m_context = 0ULL;
    uint64_t m_length = 0ULL;

    void OnAppend(const uint8_t* data, std::size_t length) override;
    std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length) override;
  };

  namespace _detail {
    std::vector<uint8_t> Sha256(const std::vector<uint8_t>& data);
    std::vector<uint8_t> HmacSha256(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& key);
    std::string UrlEncodeQueryParameter(const std::string& value);
    std::string UrlEncodePath(const std::string& value);
  } // namespace _detail
}} // namespace Azure::Storage
