// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "dll_import_export.hpp"

#include <azure/core/internal/extendable_enumeration.hpp>
namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Audiences available for table service
   *
   */
  class TablesAudience final {
    std::string m_audience;

  public:
    /**
     * @brief Get the audience value.
     */
    std::string GetAudience() const { return m_audience; }
    /**
     * @brief Construct a new TablesAudience object
     *
     * @param tablesAudience The Azure Active Directory audience to use when forming authorization
     * scopes. For the Language service, this value corresponds to a URL that identifies the Azure
     * cloud where the resource is located. For more information: See
     * https://learn.microsoft.com/azure/storage/tables/authorize-access-azure-active-directory
     */
    explicit TablesAudience(std::string tablesAudience) : m_audience{std::move(tablesAudience)} {}

    /**
     * @brief The service endpoint for a given storage account. Use this method to acquire a token
     * for authorizing requests to that specific Azure Storage account and service only.
     *
     * @param tablesAccountName The storage account name used to populate the service endpoint.
     * @return The service endpoint for a given storage account.
     */
    static TablesAudience CreateTablesServiceAccountAudience(const std::string& tablesAccountName)
    {
      return TablesAudience("https://" + tablesAccountName + ".table.core.windows.net/");
    }
  };
}}} // namespace Azure::Data::Tables
