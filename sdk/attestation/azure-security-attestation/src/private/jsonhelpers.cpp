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

#include "azure/core/base64.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/core/nullable.hpp"
#include "jsonhelpers_private.hpp"

#include <memory>
#include <string>
#include <vector>

// cspell: words jwks MrSigner MrEnclave
namespace Azure { namespace Security { namespace Attestation { namespace _detail {
  /// @brief - parse a string field from a JSON object.
  ///
  /// @param field - JSON object containing the field.
  /// @param fieldName - name of the JSON property to retrieve.
  /// @returns A `std::string` referencing the property in the JSON object, an empty string if the
  /// field does not exist.
  Azure::Nullable<std::string> JsonHelpers::ParseStringField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_string())
      {
        throw std::runtime_error("Field " + fieldName + " is not a string.");
      }
      return fieldVal.get<std::string>();
    }
    return Azure::Nullable<std::string>();
  }

  /// @brief - parse an array of strings from a JSON object.
  ///
  /// @param field - JSON object containing the field.
  /// @param fieldName - name of the JSON property to retrieve.
  /// @returns A `std::vector<std::string>` referencing the property in the JSON object, an empty
  /// array if the field does not exist.
  Azure::Nullable<std::vector<std::string>> JsonHelpers::ParseStringArrayField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    if (field.contains(fieldName))
    {
      std::vector<std::string> returnValue;
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_array())
      {
        throw std::runtime_error("Field " + fieldName + " is not an array.");
      }
      for (const auto& item : fieldVal)
      {
        if (!item.is_string())
        {
          throw std::runtime_error("Field " + fieldName + " element is not a string.");
        }
        returnValue.push_back(item.get<std::string>());
      }
      return returnValue;
    }
    return Azure::Nullable<std::vector<std::string>>();
  }

  /// @brief - parse an array of integers from a JSON object.
  ///
  /// @param field - JSON object containing the field.
  /// @param fieldName - name of the JSON property to retrieve.
  /// @returns A `std::vector<std::string>` referencing the property in the JSON object, an empty
  /// array if the field does not exist.
  Azure::Nullable<std::vector<int>> JsonHelpers::ParseIntArrayField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    std::vector<int> returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_array())
      {
        throw std::runtime_error("Field " + fieldName + " is not an array.");
      }
      for (const auto& item : fieldVal)
      {
        if (!item.is_number_integer())
        {
          throw std::runtime_error("Field " + fieldName + " element is not an integer.");
        }
        returnValue.push_back(item.get<int>());
      }
      return returnValue;
    }
    return Azure::Nullable<std::vector<int>>();
  }

  std::string JsonHelpers::ParseStringJsonField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    std::string returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_object())
      {
        throw std::runtime_error("Field " + fieldName + " is not an object.");
      }
      returnValue = field[fieldName].dump();
    }
    return returnValue;
  }

  Azure::Nullable<Azure::DateTime> JsonHelpers::ParseDateTimeField(
      Azure::Core::Json::_internal::json const& object,
      std::string const& fieldName)
  {
    if (object.contains(fieldName))
    {
      const auto& fieldVal = object[fieldName];
      if (!fieldVal.is_number())
      {
        throw std::runtime_error("Field " + fieldName + " is not a number.");
      }

      int64_t epochTime = fieldVal.get<int64_t>();
      return Azure::Core::_internal::PosixTimeConverter::PosixTimeToDateTime(epochTime);
    }
    return Azure::Nullable<Azure::DateTime>();
  }

  std::vector<uint8_t> JsonHelpers::ParseBase64UrlField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    std::vector<uint8_t> returnValue;
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_string())
      {
        throw std::runtime_error(std::string("Field ") + fieldName + " is not a string.");
      }
      returnValue
          = Azure::Core::_internal::Base64Url::Base64UrlDecode(field[fieldName].get<std::string>());
    }
    return returnValue;
  }

  Azure::Nullable<bool> JsonHelpers::ParseBooleanField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_boolean())
      {
        throw std::runtime_error("Field " + fieldName + " is not a boolean.");
      }
      return field[fieldName].get<bool>();
    }
    return Azure::Nullable<bool>();
  }

  Azure::Nullable<int> JsonHelpers::ParseIntNumberField(
      const Azure::Core::Json::_internal::json& field,
      const std::string& fieldName)
  {
    if (field.contains(fieldName))
    {
      const auto& fieldVal = field[fieldName];
      if (!fieldVal.is_number_integer())
      {
        throw std::runtime_error("Field " + fieldName + " is not a number.");
      }
      return field[fieldName].get<int>();
    }
    return Azure::Nullable<int>();
  }

  // Serialization helpers.

  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      std::string const& fieldValue,
      std::string const& fieldName)
  {
    object[fieldName] = fieldValue;
  }
  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::Nullable<std::string> const& fieldValue,
      std::string const& fieldName)
  {
    if (fieldValue.HasValue())
    {
      SetField(object, fieldValue.Value(), fieldName);
    }
  }

  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      std::vector<std::string> const& fieldValue,
      std::string const& fieldName)
  {
    object[fieldName] = fieldValue;
  }

  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::Nullable<std::vector<std::string>> const& fieldValue,
      std::string const& fieldName)
  {
    if (fieldValue.HasValue())
    {
      SetField(object, fieldValue.Value(), fieldName);
    }
  }

  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      int fieldValue,
      std::string const& fieldName)
  {
    object[fieldName] = fieldValue;
  }
  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::Nullable<int> const& fieldValue,
      std::string const& fieldName)
  {
    if (fieldValue.HasValue())
    {
      SetField(object, fieldValue.Value(), fieldName);
    }
  }
  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      std::vector<int> const& fieldValue,
      std::string const& fieldName)
  {
    if (!fieldValue.empty())
    {
      object[fieldName] = fieldValue;
    }
  }
  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::Nullable<std::vector<int>> const& fieldValue,
      std::string const& fieldName)
  {
    if (fieldValue.HasValue())
    {
      SetField(object, fieldValue.Value(), fieldName);
    }
  }

  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::Nullable<Azure::DateTime> const& fieldValue,
      std::string const& fieldName)
  {
    if (fieldValue.HasValue())
    {
      SetField(object, fieldValue.Value(), fieldName);
    }
  }
  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::DateTime const& fieldValue,
      std::string const& fieldName)
  {
    object[fieldName] = Azure::Core::_internal::PosixTimeConverter::DateTimeToPosixTime(fieldValue);
  }

  void JsonHelpers::SetField(
      Azure::Core::Json::_internal::json& object,
      Azure::Core::Json::_internal::json& fieldValue,
      std::string const& fieldName)
  {
    object[fieldName] = fieldValue;
  }
}}}} // namespace Azure::Security::Attestation::_detail