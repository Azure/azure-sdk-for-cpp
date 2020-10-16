// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Storage {

  std::string Base64Encode(const std::string& text);
  std::string Base64Decode(const std::string& text);

  class Md5 {
  public:
    Md5();
    ~Md5();

    void Update(const uint8_t* data, std::size_t length);

    std::string Digest() const;

    static std::string Hash(const uint8_t* data, std::size_t length)
    {
      Md5 instance;
      instance.Update(data, length);
      return instance.Digest();
    }

    static std::string Hash(const std::string& data)
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

    std::string Digest() const;

    static std::string Hash(const uint8_t* data, std::size_t length)
    {
      Crc64 instance;
      instance.Update(data, length);
      return instance.Digest();
    }

    static std::string Hash(const std::string& data)
    {
      return Hash(reinterpret_cast<const uint8_t*>(data.data()), data.length());
    }

  private:
    uint64_t m_context = 0ULL;
    uint64_t m_length = 0ULL;
  };

  namespace Details {
    std::string Sha256(const std::string& text);
    std::string HmacSha256(const std::string& text, const std::string& key);
    std::string UrlEncodeQueryParameter(const std::string& value);
    std::string UrlEncodePath(const std::string& value);
  } // namespace Details
}} // namespace Azure::Storage
