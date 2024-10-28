// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/dll_import_export.hpp"
#include "azure/data/tables/enum_operators.hpp"
#include "azure/data/tables/internal/constants.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Data { namespace Tables {
  class TableServiceClient;
  class TableClient;

  /**
   * @brief API version for Tables service.
   */
  class ServiceVersion final {
  public:
    /**
     * @brief Construct a new Service Version object
     *
     * @param version The string version for Tables Service.
     */
    explicit ServiceVersion(std::string version) : m_version{std::move(version)} {}

    /**
     * @brief Enable comparing between two versions.
     *
     * @param other Another service version to be compared.
     */
    bool operator==(const ServiceVersion& other) const { return m_version == other.m_version; }

    /**
     * @brief Enable comparing between two versions.
     *
     * @param other Another service version to be compared.
     */
    bool operator!=(const ServiceVersion& other) const { return !(*this == other); }

    /**
     * @brief Returns string representation.
     *
     */
    std::string const& ToString() const { return m_version; }

    /**
     * @brief API version 2019-12-12.
     *
     */
    AZ_DATA_TABLES_DLLEXPORT const static ServiceVersion V2019_02_02;

  private:
    std::string m_version;
  };

  /**
   * @brief Audiences available for Blobs
   *
   */
  class TablesAudience final
      : public Azure::Core::_internal::ExtendableEnumeration<TablesAudience> {
  public:
    /**
     * @brief Construct a new TablesAudience object
     *
     * @param tablesAudience The Azure Active Directory audience to use when forming authorization
     * scopes. For the Language service, this value corresponds to a URL that identifies the Azure
     * cloud where the resource is located. For more information: See
     * https://learn.microsoft.com/en-us/azure/storage/blobs/authorize-access-azure-active-directory
     */
    explicit TablesAudience(std::string tablesAudience)
        : ExtendableEnumeration(std::move(tablesAudience))
    {
    }
  };

  /**
   * @brief Optional parameters for constructing a new TableClient.
   */
  struct TableClientOptions final : Azure::Core::_internal::ClientOptions
  {
    /**
     * API version used by this client.
     */
    ServiceVersion ApiVersion{_detail::ApiVersion};

    /**
     * Enables tenant discovery through the authorization challenge when the client is configured
     * to use a TokenCredential. When enabled, the client will attempt an initial un-authorized
     * request to prompt a challenge in order to discover the correct tenant for the resource.
     */
    bool EnableTenantDiscovery = false;

    /**
     * The Audience to use for authentication with Azure Active Directory (AAD).
     *
     */
    Azure::Nullable<TablesAudience> Audience;
  };
}}} // namespace Azure::Data::Tables
