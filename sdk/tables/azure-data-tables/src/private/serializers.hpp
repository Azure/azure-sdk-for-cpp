// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/models.hpp"
#include "xml_wrapper.hpp"

#include <azure/core/internal/json/json.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables { namespace _detail {

  /**
   * @brief Serializers for TableService operations.
   *
   */
  class Serializers final {
  public:
    /**
     * @brief Serialize a TableEntity object into a JSON string for Create Entity operation.
     *
     */
    static std::string const CreateEntity(Models::TableEntity const& tableEntity);

    /**
     * @brief Serialize a TableEntity object into a JSON string for Merge Entity operation.
     *
     */
    static std::string const MergeEntity(Models::TableEntity const& tableEntity);

    /**
     * @brief Serialize a TableEntity object into a JSON string for Update Entity operation.
     *
     */
    static std::string const UpdateEntity(Models::TableEntity const& tableEntity);

    /**
     * @brief Serialize a TableAccessPolicy object into XML.
     *
     */
    static std::string const SetAccessPolicy(Models::TableAccessPolicy const& tableAccessPolicy);

    /**
     * @brief Deserialize a TableAccessPolicy from XML.
     *
     */
    static Models::TableAccessPolicy TableAccessPolicyFromXml(std::vector<uint8_t> responseData);

    /**
     * @brief Serialize a TableEntity object into a XML string for Create operation.
     *
     */
    static std::string const Create(std::string const& tableName);

    /**
     * @brief Serialize Service properties into XML.
     *
     */
    static std::string const SetServiceProperties(
        Models::SetServicePropertiesOptions const& options);

    /**
     * @brief Deserialize TableServiceProprties from XML.
     *
     */
    static Models::TableServiceProperties ServicePropertiesFromXml(
        std::vector<uint8_t> responseData);

    /**
     * @brief Deserialize a TableEntity from JSON.
     */
    static Models::TableEntity DeserializeEntity(Azure::Core::Json::_internal::json json);
  };
}}}} // namespace Azure::Data::Tables::_detail
