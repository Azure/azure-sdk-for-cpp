//  Copyright (c) Microsoft Corporation. All rights reserved.
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

#pragma once

#include "azure/attestation/attestation_client.hpp"
#include <azure/core/datetime.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/nullable.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {
  class JsonHelpers {
  private:
    static uint8_t FromHexChar(char hex);

  public:
    static std::vector<uint8_t> HexStringToBinary(std::string const& hexString);
    static std::string BinaryToHexString(std::vector<uint8_t> const& src);

    static std::map<std::string, std::string> DecorateAttestationData(
        Azure::Security::Attestation::AttestationData const& data);

    static void SetIfExistsJson(
        Azure::Nullable<std::string>& rv,
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);
  };
}}}} // namespace Azure::Security::Attestation::_detail
