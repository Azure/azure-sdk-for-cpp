// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "azure/data/tables/constants.hpp"

#include <azure/core/internal/client_options.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>

namespace Azure { namespace Data { namespace Tables {
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
    std::string ApiVersion{"2019-02-02"};

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
