// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/crypt.hpp"

#include "common/storage_common.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <bcrypt.h>
#else
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#endif

#include <stdexcept>

namespace Azure { namespace Storage {

#ifdef _WIN32

  namespace {

    enum class AlgorithmType
    {
      Hmac_Sha256,
      Sha256,
    };

    struct AlgorithmProviderInstance
    {
      BCRYPT_ALG_HANDLE Handle;
      std::size_t ContextSize;
      std::size_t HashLength;

      AlgorithmProviderInstance(AlgorithmType type)
      {
        const wchar_t* algorithmId = BCRYPT_SHA256_ALGORITHM;
        unsigned long algorithmFlags = 0;
        if (type == AlgorithmType::Hmac_Sha256)
        {
          algorithmFlags = BCRYPT_ALG_HANDLE_HMAC_FLAG;
        }
        NTSTATUS status
            = BCryptOpenAlgorithmProvider(&Handle, algorithmId, nullptr, algorithmFlags);
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

  } // namespace

  std::string Sha256(const std::string& text)
  {
    static AlgorithmProviderInstance AlgorithmProvider(AlgorithmType::Sha256);

    std::string context;
    context.resize(AlgorithmProvider.ContextSize);

    BCRYPT_HASH_HANDLE hashHandle;
    NTSTATUS status = BCryptCreateHash(
        AlgorithmProvider.Handle,
        &hashHandle,
        reinterpret_cast<PUCHAR>(&context[0]),
        static_cast<ULONG>(context.size()),
        nullptr,
        0,
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed");
    }

    status = BCryptHashData(
        hashHandle,
        reinterpret_cast<PBYTE>(const_cast<char*>(&text[0])),
        static_cast<ULONG>(text.length()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed");
    }

    std::string hash;
    hash.resize(AlgorithmProvider.HashLength);
    status = BCryptFinishHash(
        hashHandle, reinterpret_cast<PUCHAR>(&hash[0]), static_cast<ULONG>(hash.length()), 0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptFinishHash failed");
    }

    BCryptDestroyHash(hashHandle);

    return hash;
  }

  std::string Hmac_Sha256(const std::string& text, const std::string& key)
  {

    static AlgorithmProviderInstance AlgorithmProvider(AlgorithmType::Hmac_Sha256);

    std::string context;
    context.resize(AlgorithmProvider.ContextSize);

    BCRYPT_HASH_HANDLE hashHandle;
    NTSTATUS status = BCryptCreateHash(
        AlgorithmProvider.Handle,
        &hashHandle,
        reinterpret_cast<PUCHAR>(&context[0]),
        static_cast<ULONG>(context.size()),
        reinterpret_cast<PUCHAR>(const_cast<char*>(&key[0])),
        static_cast<ULONG>(key.length()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed");
    }

    status = BCryptHashData(
        hashHandle,
        reinterpret_cast<PBYTE>(const_cast<char*>(&text[0])),
        static_cast<ULONG>(text.length()),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed");
    }

    std::string hash;
    hash.resize(AlgorithmProvider.HashLength);
    status = BCryptFinishHash(
        hashHandle, reinterpret_cast<PUCHAR>(&hash[0]), static_cast<ULONG>(hash.length()), 0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptFinishHash failed");
    }

    BCryptDestroyHash(hashHandle);

    return hash;
  }

  std::string Base64Encode(const std::string& text)
  {
    std::string encoded;
    // According to RFC 4648, the encoded length should be ceiling(n / 3) * 4
    DWORD encodedLength = static_cast<DWORD>((text.length() + 2) / 3 * 4);
    encoded.resize(encodedLength);

    CryptBinaryToStringA(
        reinterpret_cast<const BYTE*>(text.data()),
        static_cast<DWORD>(text.length()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
        static_cast<LPSTR>(&encoded[0]),
        &encodedLength);

    return encoded;
  }

  std::string Base64Decode(const std::string& text)
  {
    std::string decoded;
    // According to RFC 4648, the encoded length should be ceiling(n / 3) * 4, so we can infer an
    // upper bound here
    DWORD decodedLength = DWORD(text.length() / 4 * 3);
    decoded.resize(decodedLength);

    CryptStringToBinaryA(
        text.data(),
        static_cast<DWORD>(text.length()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_STRICT,
        reinterpret_cast<BYTE*>(&decoded[0]),
        &decodedLength,
        nullptr,
        nullptr);
    decoded.resize(decodedLength);
    return decoded;
  }

#else

  std::string Sha256(const std::string& text)
  {
    SHA256_CTX context;
    SHA256_Init(&context);
    SHA256_Update(&context, text.data(), text.length());
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &context);
    return std::string(std::begin(hash), std::end(hash));
  }

  std::string Hmac_Sha256(const std::string& text, const std::string& key)
  {
    char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;
    HMAC(
        EVP_sha256(),
        key.data(),
        static_cast<int>(key.length()),
        reinterpret_cast<const unsigned char*>(text.data()),
        text.length(),
        reinterpret_cast<unsigned char*>(&hash[0]),
        &hashLength);

    return std::string(hash, hashLength);
  }

  std::string Base64Encode(const std::string& text)
  {
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(BIO_new(BIO_f_base64()), bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, text.data(), static_cast<int>(text.length()));
    BIO_flush(bio);
    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    return std::string(bufferPtr->data, bufferPtr->length);
  }

  std::string Base64Decode(const std::string& text)
  {
    std::string decoded;
    // According to RFC 4648, the encoded length should be ceiling(n / 3) * 4, so we can infer an
    // upper bound here
    std::size_t maxDecodedLength = text.length() / 4 * 3;
    decoded.resize(maxDecodedLength);

    BIO* bio = BIO_new_mem_buf(text.data(), -1);
    bio = BIO_push(BIO_new(BIO_f_base64()), bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decodedLength = BIO_read(bio, &decoded[0], static_cast<int>(text.length()));
    BIO_free_all(bio);

    decoded.resize(decodedLength);
    return decoded;
  }

#endif

}} // namespace Azure::Storage
