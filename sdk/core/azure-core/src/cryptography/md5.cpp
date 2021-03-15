// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/cryptography/hash.hpp"
#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
// Windows needs to go before bcrypt
#include <windows.h>

#include <bcrypt.h>
#elif defined(AZ_PLATFORM_POSIX)
#include <openssl/md5.h>
#endif

#include <stdexcept>
#include <vector>

namespace Azure { namespace Core { namespace Cryptography {

#if defined(AZ_PLATFORM_WINDOWS)

  namespace _detail {
    struct AlgorithmProviderInstance
    {
      BCRYPT_ALG_HANDLE Handle;
      std::size_t ContextSize;
      std::size_t HashLength;

      AlgorithmProviderInstance()
      {
        NTSTATUS status = BCryptOpenAlgorithmProvider(&Handle, BCRYPT_MD5_ALGORITHM, nullptr, 0);
        if (!BCRYPT_SUCCESS(status))
        {
          throw std::runtime_error("BCryptOpenAlgorithmProvider failed");
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
          throw std::runtime_error("BCryptGetProperty failed");
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
          throw std::runtime_error("BCryptGetProperty failed");
        }
        HashLength = hashLength;
      }

      ~AlgorithmProviderInstance() { BCryptCloseAlgorithmProvider(Handle, 0); }
    };

    struct Md5HashContext
    {
      std::string buffer;
      BCRYPT_HASH_HANDLE hashHandle = nullptr;
      std::size_t hashLength = 0;
    };
  } // namespace _detail

  Md5Hash::Md5Hash()
  {
    static _detail::AlgorithmProviderInstance AlgorithmProvider{};

    _detail::Md5HashContext* md5Context = new _detail::Md5HashContext;
    m_md5Context = md5Context;
    md5Context->buffer.resize(AlgorithmProvider.ContextSize);
    md5Context->hashLength = AlgorithmProvider.HashLength;

    NTSTATUS status = BCryptCreateHash(
        AlgorithmProvider.Handle,
        &md5Context->hashHandle,
        reinterpret_cast<PUCHAR>(&md5Context->buffer[0]),
        static_cast<ULONG>(md5Context->buffer.size()),
        nullptr,
        0,
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed");
    }
  }

  Md5Hash::~Md5Hash()
  {
    _detail::Md5HashContext* md5Context = static_cast<_detail::Md5HashContext*>(m_md5Context);
    BCryptDestroyHash(md5Context->hashHandle);
    delete md5Context;
  }

  void Md5Hash::OnAppend(const uint8_t* data, std::size_t length)
  {
    _detail::Md5HashContext* md5Context = static_cast<_detail::Md5HashContext*>(m_md5Context);

    NTSTATUS status = BCryptHashData(
        md5Context->hashHandle,
        reinterpret_cast<PBYTE>(const_cast<uint8_t*>(data)),
        static_cast<ULONG>(length),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed");
    }
  }

  std::vector<uint8_t> Md5Hash::OnFinal(const uint8_t* data, std::size_t length)
  {
    OnAppend(data, length);
    _detail::Md5HashContext* md5Context = static_cast<_detail::Md5HashContext*>(m_md5Context);
    std::vector<uint8_t> hash;
    hash.resize(md5Context->hashLength);
    NTSTATUS status = BCryptFinishHash(
        md5Context->hashHandle,
        reinterpret_cast<PUCHAR>(&hash[0]),
        static_cast<ULONG>(hash.size()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptFinishHash failed");
    }
    return hash;
  }

#elif defined(AZ_PLATFORM_POSIX)

  Md5Hash::Md5Hash()
  {
    MD5_CTX* md5Context = new MD5_CTX;
    m_md5Context = md5Context;
    MD5_Init(md5Context);
  }

  Md5Hash::~Md5Hash()
  {
    MD5_CTX* md5Context = static_cast<MD5_CTX*>(m_md5Context);
    delete md5Context;
  }

  void Md5Hash::OnAppend(const uint8_t* data, std::size_t length)
  {
    MD5_CTX* md5Context = static_cast<MD5_CTX*>(m_md5Context);
    MD5_Update(md5Context, data, length);
  }

  std::vector<uint8_t> Md5Hash::OnFinal(const uint8_t* data, std::size_t length)
  {
    OnAppend(data, length);
    MD5_CTX* md5Context = static_cast<MD5_CTX*>(m_md5Context);
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, md5Context);
    return std::vector<uint8_t>(std::begin(hash), std::end(hash));
  }

#endif
}}} // namespace Azure::Core::Cryptography
