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

#pragma once

#include "azure/core/base64.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/core/nullable.hpp"

#include <memory>
#include <string>
#include <vector>

// cspell: words jwks MrSigner MrEnclave
namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  struct JsonHelpers
  {
    /// @brief - parse a string field from a JSON object.
    ///
    /// @param field - JSON object containing the field.
    /// @param fieldName - name of the JSON property to retrieve.
    /// @returns A `std::string` referencing the property in the JSON object, an empty string if the
    /// field does not exist.
    static Azure::Nullable<std::string> ParseStringField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    /// @brief - parse an array of strings from a JSON object.
    ///
    /// @param field - JSON object containing the field.
    /// @param fieldName - name of the JSON property to retrieve.
    /// @returns A `std::vector<std::string>` referencing the property in the JSON object, an empty
    /// array if the field does not exist.
    static Azure::Nullable<std::vector<std::string>> ParseStringArrayField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    /// @brief - parse an array of integers from a JSON object.
    ///
    /// @param field - JSON object containing the field.
    /// @param fieldName - name of the JSON property to retrieve.
    /// @returns A `std::vector<std::string>` referencing the property in the JSON object, an empty
    /// array if the field does not exist.
    static Azure::Nullable<std::vector<int>> ParseIntArrayField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    static std::string ParseStringJsonField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    static Azure::Nullable<Azure::DateTime> ParseDateTimeField(
        Azure::Core::Json::_internal::json const& object,
        std::string const& fieldName);

    static std::vector<uint8_t> ParseBase64UrlField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    static Azure::Nullable<bool> ParseBooleanField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    static Azure::Nullable<int> ParseIntNumberField(
        const Azure::Core::Json::_internal::json& field,
        const std::string& fieldName);

    // Serialization helpers.
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        std::string const& fieldValue,
        std::string const& fieldName);
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::Nullable<std::string> const& fieldValue,
        std::string const& fieldName);

    static void SetField(
        Azure::Core::Json::_internal::json& object,
        std::vector<std::string> const& fieldValue,
        std::string const& fieldName);

    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::Nullable<std::vector<std::string>> const& fieldValue,
        std::string const& fieldName);

    static void SetField(
        Azure::Core::Json::_internal::json& object,
        int fieldValue,
        std::string const& fieldName);
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::Nullable<int> const& fieldValue,
        std::string const& fieldName);
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        std::vector<int> const& fieldValue,
        std::string const& fieldName);
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::Nullable<std::vector<int>> const& fieldValue,
        std::string const& fieldName);

    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::Nullable<Azure::DateTime> const& fieldValue,
        std::string const& fieldName);
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::DateTime const& fieldValue,
        std::string const& fieldName);
    static void SetField(
        Azure::Core::Json::_internal::json& object,
        Azure::Core::Json::_internal::json& fieldValue,
        std::string const& fieldName);
  };
}}}} // namespace Azure::Security::Attestation::_internal