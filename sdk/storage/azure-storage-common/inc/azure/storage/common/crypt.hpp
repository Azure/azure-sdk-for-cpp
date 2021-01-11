// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <azure/core/base64.hpp>

namespace Azure { namespace Storage {

  class Md5 {
  public:
    Md5();
    ~Md5();

    void Update(const uint8_t* data, std::size_t length);

    std::vector<uint8_t> Digest() const;

    static std::vector<uint8_t> Hash(const uint8_t* data, std::size_t length)
    {
      Md5 instance;
      instance.Update(data, length);
      return instance.Digest();
    }

    static std::vector<uint8_t> Hash(const std::string& data)
    {
      return Hash(reinterpret_cast<const uint8_t*>(data.data()), data.length());
    }

  private:
    void* m_context;
  };

  class Crc64 {
  public:
    void Update(const uint8_t* data, std::size_t length);
    void Concatenate(const Crc64& other);

    std::vector<uint8_t> Digest() const;

    static std::vector<uint8_t> Hash(const uint8_t* data, std::size_t length)
    {
      Crc64 instance;
      instance.Update(data, length);
      return instance.Digest();
    }

    static std::vector<uint8_t> Hash(const std::string& data)
    {
      return Hash(reinterpret_cast<const uint8_t*>(data.data()), data.length());
    }

  private:
    uint64_t m_context = 0ULL;
    uint64_t m_length = 0ULL;
  };

  namespace Details {
    std::vector<uint8_t> Sha256(const std::vector<uint8_t>& data);
    std::vector<uint8_t> HmacSha256(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& key);
    std::string UrlEncodeQueryParameter(const std::string& value);
    std::string UrlEncodePath(const std::string& value);
  } // namespace Details
}} // namespace Azure::Storage
