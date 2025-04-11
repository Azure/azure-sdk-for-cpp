// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <string>
#include <utility>

namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Audiences available for Azure Table storage service.

   *
   */
  class TableAudience final {
    std::string m_audience;

  public:
    /**
     * @brief Get the audience value.
     */
    std::string GetAudience() const { return m_audience; }
    /**
     * @brief Construct a new TablesAudience object
     *
     * @param tableAudience The Azure Active Directory audience to use when forming authorization
     * scopes. For the Language service, this value corresponds to a URL that identifies the Azure
     * cloud where the resource is located. For more information: See
     * https://learn.microsoft.com/azure/storage/tables/authorize-access-azure-active-directory
     */
    explicit TableAudience(std::string tableAudience) : m_audience{std::move(tableAudience)} {}

    /**
     * @brief The service endpoint for a given storage account. Use this method to acquire a token
     * for authorizing requests to that specific Azure Storage account and service only.
     *
     * @param tableAccountName The storage account name used to populate the service endpoint.
     * @return The service endpoint for a given storage account.
     */
    static TableAudience CreateAccountAudience(const std::string& tableAccountName)
    {
      return TableAudience("https://" + tableAccountName + ".table.core.windows.net/");
    }
  };
}}} // namespace Azure::Data::Tables
