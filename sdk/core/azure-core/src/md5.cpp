// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/md5.hpp"
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

namespace Azure { namespace Core {

#if defined(AZ_PLATFORM_WINDOWS)

  namespace Details {
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
  } // namespace Details

  Md5::Md5()
  {
    static Details::AlgorithmProviderInstance AlgorithmProvider{};

    Details::Md5HashContext* context = new Details::Md5HashContext;
    m_context = context;
    context->buffer.resize(AlgorithmProvider.ContextSize);
    context->hashLength = AlgorithmProvider.HashLength;

    NTSTATUS status = BCryptCreateHash(
        AlgorithmProvider.Handle,
        &context->hashHandle,
        reinterpret_cast<PUCHAR>(&context->buffer[0]),
        static_cast<ULONG>(context->buffer.size()),
        nullptr,
        0,
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed");
    }
  }

  Md5::~Md5()
  {
    Details::Md5HashContext* context = static_cast<Details::Md5HashContext*>(m_context);
    BCryptDestroyHash(context->hashHandle);
    delete context;
  }

  void Md5::Update(const std::vector<uint8_t>& data)
  {
    Details::Md5HashContext* context = static_cast<Details::Md5HashContext*>(m_context);

    NTSTATUS status = BCryptHashData(
        context->hashHandle,
        reinterpret_cast<PBYTE>(const_cast<uint8_t*>(data.data())),
        static_cast<ULONG>(data.size()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed");
    }
  }

  std::vector<uint8_t> Md5::Digest() const
  {
    Details::Md5HashContext* context = static_cast<Details::Md5HashContext*>(m_context);
    std::vector<uint8_t> hash;
    hash.resize(context->hashLength);
    NTSTATUS status = BCryptFinishHash(
        context->hashHandle,
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

  Md5::Md5()
  {
    MD5_CTX* context = new MD5_CTX;
    m_context = context;
    MD5_Init(context);
  }

  Md5::~Md5()
  {
    MD5_CTX* context = static_cast<MD5_CTX*>(m_context);
    delete context;
  }

  void Md5::Update(const std::vector<uint8_t>& data)
  {
    MD5_CTX* context = static_cast<MD5_CTX*>(m_context);
    MD5_Update(context, data.data(), data.size());
  }

  std::vector<uint8_t> Md5::Digest() const
  {
    MD5_CTX* context = static_cast<MD5_CTX*>(m_context);
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, context);
    return std::vector<uint8_t>(std::begin(hash), std::end(hash));
  }

#endif
}} // namespace Azure::Core
