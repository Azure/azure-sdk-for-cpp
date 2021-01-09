// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/base64.hpp"
#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(AZ_PLATFORM_POSIX)
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#endif

#include <string>
#include <vector>

namespace Azure { namespace Core {

#if defined(AZ_PLATFORM_WINDOWS)

  std::string Base64Encode(const std::vector<uint8_t>& data)
  {
    std::string encoded;
    // According to RFC 4648, the encoded length should be ceiling(n / 3) * 4
    DWORD encodedLength = static_cast<DWORD>((data.size() + 2) / 3 * 4);
    encoded.resize(encodedLength);

    CryptBinaryToStringA(
        reinterpret_cast<const BYTE*>(data.data()),
        static_cast<DWORD>(data.size()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
        static_cast<LPSTR>(&encoded[0]),
        &encodedLength);

    return encoded;
  }

  std::vector<uint8_t> Base64Decode(const std::string& text)
  {
    std::vector<uint8_t> decoded;
    // According to RFC 4648, the encoded length should be ceiling(n / 3) * 4, so we can infer an
    // upper bound here
    DWORD decodedLength = DWORD(text.length() / 4 * 3);
    decoded.resize(decodedLength);

    CryptStringToBinaryA(
        text.data(),
        static_cast<DWORD>(text.length()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_STRICT,
        reinterpret_cast<BYTE*>(decoded.data()),
        &decodedLength,
        nullptr,
        nullptr);
    decoded.resize(decodedLength);
    return decoded;
  }

#elif defined(AZ_PLATFORM_POSIX)

  std::string Base64Encode(const std::vector<uint8_t>& data)
  {
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(BIO_new(BIO_f_base64()), bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.data(), static_cast<int>(data.size()));
    BIO_flush(bio);
    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    return std::string(bufferPtr->data, bufferPtr->length);
  }

  std::vector<uint8_t> Base64Decode(const std::string& text)
  {
    std::vector<uint8_t> decoded;
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

}} // namespace Azure::Core
