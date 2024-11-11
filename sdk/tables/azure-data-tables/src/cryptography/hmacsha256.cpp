// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../private/hmacsha256.hpp"

#include <azure/core/azure_assert.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/platform.hpp>
#if defined(AZ_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// Windows needs to go before bcrypt
#include <windows.h>

#include <bcrypt.h>
#elif defined(AZ_PLATFORM_POSIX)
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>
namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Cryptography {
#if defined(AZ_PLATFORM_WINDOWS)

  enum class AlgorithmType
  {
    HmacSha256,
  };

  struct AlgorithmProviderInstance final
  {
    BCRYPT_ALG_HANDLE Handle;
    size_t ContextSize;
    size_t HashLength;

    AlgorithmProviderInstance(AlgorithmType type)
    {
      const wchar_t* algorithmId = nullptr;
      if (type == AlgorithmType::HmacSha256)
      {
        algorithmId = BCRYPT_SHA256_ALGORITHM;
      }
      else
      {
        throw std::runtime_error("Unknown algorithm type.");
      }

      unsigned long algorithmFlags = 0;
      if (type == AlgorithmType::HmacSha256)
      {
        algorithmFlags = BCRYPT_ALG_HANDLE_HMAC_FLAG;
      }
      Handle = nullptr;
      NTSTATUS status = BCryptOpenAlgorithmProvider(&Handle, algorithmId, nullptr, algorithmFlags);
      if (!BCRYPT_SUCCESS(status))
      {
        throw std::runtime_error("BCryptOpenAlgorithmProvider failed.");
      }
      DWORD objectLength = 0;
      DWORD dataLength = 0;
      status = BCryptGetProperty(
          Handle,
          BCRYPT_OBJECT_LENGTH,
          reinterpret_cast<PBYTE>(&objectLength),
          sizeof(objectLength),
          &dataLength,
          0);
      if (!BCRYPT_SUCCESS(status))
      {
        throw std::runtime_error("BCryptGetProperty failed.");
      }
      ContextSize = objectLength;
      DWORD hashLength = 0;
      status = BCryptGetProperty(
          Handle,
          BCRYPT_HASH_LENGTH,
          reinterpret_cast<PBYTE>(&hashLength),
          sizeof(hashLength),
          &dataLength,
          0);
      if (!BCRYPT_SUCCESS(status))
      {
        throw std::runtime_error("BCryptGetProperty failed.");
      }
      HashLength = hashLength;
    }

    ~AlgorithmProviderInstance() { BCryptCloseAlgorithmProvider(Handle, 0); }
  };

  std::vector<uint8_t> HmacSha256::Compute(
      const std::vector<uint8_t>& data,
      const std::vector<uint8_t>& key)
  {
    AZURE_ASSERT_MSG(data.size() <= ULONG_MAX, "Data size is too big.");

    static AlgorithmProviderInstance AlgorithmProvider(AlgorithmType::HmacSha256);

    std::string context;
    context.resize(AlgorithmProvider.ContextSize);

    BCRYPT_HASH_HANDLE hashHandle;
    NTSTATUS status = BCryptCreateHash(
        AlgorithmProvider.Handle,
        &hashHandle,
        reinterpret_cast<PUCHAR>(&context[0]),
        static_cast<ULONG>(context.size()),
        reinterpret_cast<PUCHAR>(const_cast<uint8_t*>(&key[0])),
        static_cast<ULONG>(key.size()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed.");
    }

    status = BCryptHashData(
        hashHandle,
        reinterpret_cast<PBYTE>(const_cast<uint8_t*>(data.data())),
        static_cast<ULONG>(data.size()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed.");
    }

    std::vector<uint8_t> hash;
    hash.resize(AlgorithmProvider.HashLength);
    status = BCryptFinishHash(
        hashHandle, reinterpret_cast<PUCHAR>(&hash[0]), static_cast<ULONG>(hash.size()), 0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptFinishHash failed.");
    }

    BCryptDestroyHash(hashHandle);

    return hash;
  }

#elif defined(AZ_PLATFORM_POSIX)

  std::vector<uint8_t> HmacSha256::Compute(
      const std::vector<uint8_t>& data,
      const std::vector<uint8_t>& key)
  {
    uint8_t hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;
    HMAC(
        EVP_sha256(),
        key.data(),
        static_cast<int>(key.size()),
        reinterpret_cast<const unsigned char*>(data.data()),
        data.size(),
        reinterpret_cast<unsigned char*>(&hash[0]),
        &hashLength);

    return std::vector<uint8_t>(std::begin(hash), std::begin(hash) + hashLength);
  }

#endif
}}}}} // namespace Azure::Data::Tables::_detail::Cryptography
