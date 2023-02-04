// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Code generated by Microsoft (R) AutoRest C++ Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is
// regenerated.
#pragma once

#include <azure/core/context.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>
#include <azure/keyvault/administration/dll_import_export.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
  namespace Models {

    /**
     * @brief The type specifier of the value.
     */
    class SettingType final : public Azure::Core::_internal::ExtendableEnumeration<SettingType> {
    public:
      /**
       * @brief Default constructor
       */
      SettingType() = default;

      /**
       * @brief Constructor
       * @param value Setting Value
       */
      explicit SettingType(std::string value) : ExtendableEnumeration(std::move(value)) {}

      /*
       * @brief Specifies that this represents the Boolean Type
       */
      AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLLEXPORT const static SettingType Boolean;
    };

    /**
     * @brief Setting struct
     */
    struct Setting final
    {
      /**
       * The account setting to be updated.
       */
      std::string Name;
      /**
       * The value of the pool setting.
       */
      std::string Value;
      /**
       * The type specifier of the value.
       */
      Azure::Nullable<SettingType> Type;
    };

    /**
     * @brief The settings list result.
     */
    struct SettingsListResult final
    {
      /**
       * A response message containing a list of account settings with their
       * associated value.
       */
      std::vector<Setting> Value;
    };

}}}}} // namespace Azure::Security::KeyVault::Administration::Models
