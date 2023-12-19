// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>
#include <azure/data/tables/dll_import_export.hpp>
#include <azure/data/tables/models.hpp>
#include <azure/data/tables/rtti.hpp>
#include <azure/data/tables/tables_clients.hpp>
#include <azure/data/tables/transactions.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy_lite.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables {
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
     * @brief Serialize a TableEntity object into a JSON string for Upsert Entity operation.
     *
     */
    static std::string const SetAccessPolicy(Models::TableAccessPolicy const& tableAccessPolicy);

    /**
     * @brief Serialize a TableEntity object into a XML string for Create operation.
     *
     */
    static std::string const Create(std::string const& tableName);

    /**
     * @brief Serialize a TableEntity object into a XML string for Set service properties operation.
     *
     */
    static std::string const SetServiceProperties(
        Models::SetServicePropertiesOptions const& options);
  };
}}} // namespace Azure::Data::Tables
