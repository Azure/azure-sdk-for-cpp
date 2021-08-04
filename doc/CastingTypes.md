# Casting Types

Using c-casting style must not be used within the code implementation. Below type-castings can be found in the SDK code which have been reviewed and approved as safe convertions.

All type-casting statements are expected to include a comment to this file and the types should been verify and included to this list.

## static cast

## dynamic cast

## const cast

## reinterpret cast

- `void*` to `T*` : Safe to use in Context class after doing a assertion on the typeid. Usages:

  - Context : to cast the void\* type to the T value stored in the Context.

- `DWORD\*` to `PBYTE` : ToBeConfirmed. Usages:

  - md5.cpp : on Windows, calling `BCryptGetProperty` and `CryptStringToBinaryA`.

  - sha_hash.cpp : on Windows, callng `BCryptGetProperty`.

  - crypt.cpp : on Windows, callng `BCryptGetProperty`.

- `char\*` to `PUCHAR` : ToBeConfirmed. Usages:

  - md5.cpp : on Windows, calling `BCryptCreateHash`.

  - sha_hash.cpp : on Windows, calling `BCryptCreateHash`.

  - crypt.cpp : on Windows, callng `BCryptCreateHash`.

- `const char\*` to `const uint8_t\*` : uint8_t is equivalent in size to char. Allowed. Usages:

  - curl.cpp :

    - `SetHeader` from std::string.
    - `CreateHTTPResponse` from std::string.
    - `SendBuffer` from std::string.

  - win_http_transport.cpp :

    - `SetHeader` from std::string.

  - token_credential_impl.hpp :

    - `TokenRequest` creating memoryBodyStream from std::string.

  - Creating memoryBodyStream from std::string several times:

    - blob_rest_client.hpp
    - share_rest_client.hpp
    - queue_rest_client.hpp

- `const xmlChar\*` to `const char\*` : ToBeConfirmed. Usages:

  - xml_wrapper.cpp : Multiple occurrences.

- `const char\*` to `const xmlChar\*` : ToBeConfirmed. Usages:

  - xml_wrapper.cpp : Multiple occurrences.

- `const char\*` to `xmlChar\*` : ToBeConfirmed. Usages:

  - xml_wrapper.cpp.

- `const uint8_t*` to `const BYTE*` : Allowed it as long as BYTE is 8bit, which is true on Windows.h. Usages:

  - base64.cpp : on Windows, need to call `CryptBinaryToStringA`.

- `const uint8_t\*` to `const char\*` : uint8_t is equivalent in size to char. Allowed. Usages:

  - Parsing xml from ResponseBody as char\* + length in several places :

    - blob_rest_client.hpp.
    - share_rest_client.hpp.
    - queue_rest_client.hpp.

- `const uint8_t\*` to `const uint64_t\*` : Fine as there's no overflow. Allowed. Usages:

  - crypt.cpp : on Windows, callng `Crc64Hash::OnAppend`.

- `uint8_t\*` to `char\*` : uint8_t is equivalent in size to char. Allowed. Usages:

  - curl.cpp :

    - `ParseChunkSize` Appending data from uint8_t array to std::string.

- `uint8_t\*` to `const char\*` : uint8_t is equivalent in size to char. Allowed. Usages:

  - storage_exception : Parsing XML from bodyBuffer, which is uint8_t as const char in `CreateFromResponse`.

- `uint8_t\*` to `PBYTE` : ToBeConfirmed. Usages:

  - md5.cpp : on Windows, calling `BCryptHashData`.

  - sha_hash.cpp : on Windows, calling `BCryptHashData`.

  - crypt.cpp : on Windows, callng `BCryptHashData`.

- `uint8_t\*` to `PUCHAR` : ToBeConfirmed. Usages:

  - md5.cpp : on Windows, calling `BCryptFinishHash`.

  - sha_hash.cpp : on Windows, calling `BCryptFinishHash`.

  - crypt.cpp : on Windows, callng `BCryptCreateHash` and `BCryptFinishHash`.

- `uint8_t\*` to `const unsigned char\*` : ToBeConfirmed. Usages:

  - crypt.cpp : on Posix, callng `HmacSha256`.

- `uint8_t\*` to `unsigned char\*` : ToBeConfirmed. Usages:

  - crypt.cpp : on Posix, callng `HmacSha256`.
