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
#endif

#include <stdexcept>

namespace Azure { namespace Storage {

#ifdef _WIN32
  std::string HMAC_SHA256(const std::string& text, const std::string& key)
  {
    struct AlgorithmProviderInstance
    {
      BCRYPT_ALG_HANDLE Handle;
      std::size_t ContextSize;
      std::size_t HashLength;

      AlgorithmProviderInstance()
      {
        NTSTATUS status = BCryptOpenAlgorithmProvider(
            &Handle, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
        if (!BCRYPT_SUCCESS(status))
        {
          throw std::runtime_error("BCryptOpenAlgorithmProvider failed");
        }
        DWORD objectLength = 0;
        DWORD dataLength = 0;
        status = BCryptGetProperty(
            Handle,
            BCRYPT_OBJECT_LENGTH,
            (PBYTE)&objectLength,
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
            Handle, BCRYPT_HASH_LENGTH, (PBYTE)&hashLength, sizeof(hashLength), &dataLength, 0);
        if (!BCRYPT_SUCCESS(status))
        {
          throw std::runtime_error("BCryptGetProperty failed");
        }
        HashLength = hashLength;
      }

      ~AlgorithmProviderInstance() { BCryptCloseAlgorithmProvider(Handle, 0); }
    };

    static AlgorithmProviderInstance AlgorithmProvider;

    std::string context;
    context.resize(AlgorithmProvider.ContextSize);

    BCRYPT_HASH_HANDLE hashHandle;
    NTSTATUS status = BCryptCreateHash(
        AlgorithmProvider.Handle,
        &hashHandle,
        (PUCHAR)context.data(),
        (ULONG)context.size(),
        (PUCHAR)key.data(),
        (ULONG)key.length(),
        0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptCreateHash failed");
    }

    status = BCryptHashData(hashHandle, (PBYTE)text.data(), (ULONG)text.length(), 0);
    if (!BCRYPT_SUCCESS(status))
    {
      throw std::runtime_error("BCryptHashData failed");
    }

    std::string hash;
    hash.resize(AlgorithmProvider.HashLength);
    status = BCryptFinishHash(hashHandle, (PUCHAR)hash.data(), (ULONG)hash.length(), 0);
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
    DWORD encodedLength = DWORD((text.length() + 2) / 3 * 4);
    encoded.resize(encodedLength);

    CryptBinaryToStringA(
        (BYTE*)text.data(),
        (DWORD)text.length(),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
        (LPSTR)encoded.data(),
        (DWORD*)&encodedLength);

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
        (DWORD)text.length(),
        CRYPT_STRING_BASE64 | CRYPT_STRING_STRICT,
        (BYTE*)decoded.data(),
        &decodedLength,
        NULL,
        NULL);
    decoded.resize(decodedLength);
    return decoded;
  }

#else

  std::string HMAC_SHA256(const std::string& text, const std::string& key)
  {
    char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;
    HMAC(
        EVP_sha256(),
        key.data(),
        (int)key.length(),
        (const unsigned char*)text.data(),
        text.length(),
        (unsigned char*)&hash[0],
        &hashLength);

    return std::string(hash, hashLength);
  }

  std::string Base64Encode(const std::string& text)
  {
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(BIO_new(BIO_f_base64()), bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, text.data(), (int)text.length());
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
    int decodedLength = BIO_read(bio, &decoded[0], (int)text.length());
    BIO_free_all(bio);

    decoded.resize(decodedLength);
    return decoded;
  }
#endif

}} // namespace Azure::Storage
