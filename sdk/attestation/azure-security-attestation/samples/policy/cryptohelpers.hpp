//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <memory>
#include <string>
#include <vector>
/**
 * @brief THe Cryptography class provides a set of basic cryptographic primatives required
 * by the attestation samples.
 */
class Cryptography {
public:
  /**
   * @brief Convert a base64 encoded value to the PEM encoded equivalent.
   *
   * @param base64 base64 encoded value.
   * @param pemType Type of the object to be converted - typically "CERTIFICATE", "PRIVATE KEY",
   * or "PUBLIC KEY".
   * @return std::string PEM encoded representation of the base64 value.
   */
  static std::string PemFromBase64(std::string const& base64, std::string const& pemType)
  {
    std::string rv;
    rv += "-----BEGIN ";
    rv += pemType;
    rv += "-----\r\n ";
    std::string encodedValue(base64);

    // Insert crlf characters every 80 characters into the base64 encoded key to make it
    // prettier.
    size_t insertPos = 80;
    while (insertPos < encodedValue.length())
    {
      encodedValue.insert(insertPos, "\r\n");
      insertPos += 82; /* 80 characters plus the \r\n we just inserted */
    }

    rv += encodedValue;
    rv += "\r\n-----END ";
    rv += pemType;
    rv += "-----\r\n ";
    return rv;
  }
};
