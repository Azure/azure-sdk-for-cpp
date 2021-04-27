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

namespace {

#if defined(AZ_PLATFORM_WINDOWS)

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

class Md5BCrypt : public Azure::Core::Cryptography::Hash {
private:
  std::string m_buffer;
  BCRYPT_HASH_HANDLE m_hashHandle = nullptr;
  std::size_t m_hashLength = 0;
  AlgorithmProviderInstance m_algorithmProviderInstance;

  void OnAppend(const uint8_t* data, std::size_t length)
  {
    NTSTATUS status = BCryptHashData(
        m_hashHandle,
        reinterpret_cast<PBYTE>(const_cast<uint8_t*>(data)),
        static_cast<ULONG>(length),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed");
    }
  }

  std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length)
  {
    OnAppend(data, length);

    std::vector<uint8_t> hash;
    hash.resize(m_hashLength);
    NTSTATUS status = BCryptFinishHash(
        m_hashHandle, reinterpret_cast<PUCHAR>(&hash[0]), static_cast<ULONG>(hash.size()), 0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptFinishHash failed");
    }
    return hash;
  }

public:
  Md5BCrypt()
  {
    m_buffer.resize(m_algorithmProviderInstance.ContextSize);
    m_hashLength = m_algorithmProviderInstance.HashLength;

    NTSTATUS status = BCryptCreateHash(
        m_algorithmProviderInstance.Handle,
        &m_hashHandle,
        reinterpret_cast<PUCHAR>(&m_buffer[0]),
        static_cast<ULONG>(m_buffer.size()),
        nullptr,
        0,
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed");
    }
  }
  ~Md5BCrypt() {}
};

} // namespace
Azure::Core::Cryptography::Md5Hash::Md5Hash() : m_implementation(std::make_unique<Md5BCrypt>()) {}

#elif defined(AZ_PLATFORM_POSIX)

class Md5OpenSSL : public Azure::Core::Cryptography::Hash {
private:
  std::unique_ptr<MD5_CTX> m_context;

  void OnAppend(const uint8_t* data, std::size_t length)
  {
    MD5_Update(m_context.get(), data, length);
  }

  std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length)
  {
    OnAppend(data, length);
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, m_context.get());
    return std::vector<uint8_t>(std::begin(hash), std::end(hash));
  }

public:
  Md5OpenSSL()
  {
    m_context = std::make_unique<MD5_CTX>();
    MD5_Init(m_context.get());
  }
};

} // namespace
Azure::Core::Cryptography::Md5Hash::Md5Hash() : m_implementation(std::make_unique<Md5OpenSSL>()) {}
#endif

namespace Azure { namespace Core { namespace Cryptography {
  Md5Hash::~Md5Hash() {}

  void Md5Hash::OnAppend(const uint8_t* data, std::size_t length)
  {
    m_implementation->Append(data, length);
  }

  std::vector<uint8_t> Md5Hash::OnFinal(const uint8_t* data, std::size_t length)
  {
    return m_implementation->Final(data, length);
  }

}}} // namespace Azure::Core::Cryptography
