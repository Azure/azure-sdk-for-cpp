// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Azure::Core::Json::_internal::json Deserialization support functions.
 *
 * This file contains a set of support functions to aid in serializing and deserializing JSON
 * objects. It also contains Deserializer classes, one for each model type which each support a
 * static Serialize and Deserialize function which serialize and deserialize the specified model
 * types from and to JSON objects.
 *
 */

#include "jsonhelpers_private.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/nullable.hpp>

#include <memory>
#include <string>
#include <vector>

// cspell: words jwks MrSigner MrEnclave
namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  using namespace Azure::Core::Json::_internal;
  void JsonHelpers::SetIfExistsJson(
      Azure::Nullable<std::string>& returnValue,
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    if (field.contains(fieldName))
    {
      returnValue = field[fieldName].dump();
    }
  }

  std::string JsonHelpers::BinaryToHexString(std::vector<uint8_t> const& src)
  {
    static constexpr char hexMap[]
        = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    std::string output(static_cast<size_t>(src.size()) * 2, ' ');
    const uint8_t* input = src.data();

    for (size_t i = 0; i < src.size(); i++)
    {
      output[2 * i] = hexMap[(input[i] & 0xF0) >> 4];
      output[2 * i + 1] = hexMap[input[i] & 0x0F];
    }

    return output;
  }

  constexpr uint8_t JsonHelpers::FromHexChar(char hex)
  {
    uint8_t val = 0;
    if (hex >= '0' && hex <= '9')
    {
      val = hex - '0';
    }
    else if (hex >= 'a' && hex <= 'f')
    {
      val = hex - 'a' + 10;
    }
    else if (hex >= 'A' && hex <= 'F')
    {
      val = hex - 'A' + 10;
    }
    else
    {
      throw std::runtime_error("Invalid character presented to FromHexChar");
    }
    return val;
  }

  std::vector<uint8_t> JsonHelpers::HexStringToBinary(std::string const& hexString)
  {
    if (hexString.size() % 2 != 0)
    {
      throw std::invalid_argument("FromHexString called with an odd length string.");
    }

    std::vector<uint8_t> decodedBuffer;
    for (int i = 0; i < static_cast<int>(hexString.size()); i += 2)
    {
      uint8_t first = FromHexChar(hexString[i]);
      uint8_t second = FromHexChar(hexString[i + 1]);
      decodedBuffer.push_back(static_cast<uint8_t>((first << 4) + second));
    }

    return decodedBuffer;
  }
}}}} // namespace Azure::Security::Attestation::_detail