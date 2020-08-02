// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/access_conditions.hpp"
#include "nullable.hpp"
#include "protocol/share_rest_client.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  /**
   * @brief Service client options used to initalize ServiceClient.
   */
  struct ServiceClientOptions
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;
  };

  struct ListSharesOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Filters the results to return only entries whose name begins with the specified
     * prefix.
     */
    Azure::Core::Nullable<std::string> Prefix;

    /**
     * @brief  A string value that identifies the portion of the list to be returned with the next
     * list operation. The operation returns a marker value within the response body if the list
     * returned was not complete. The marker value may then be used in a subsequent call to request
     * the next set of list items. The marker value is opaque to the client.
     */
    Azure::Core::Nullable<std::string> Marker;

    /**
     * @brief Specifies the maximum number of entries to return. If the request does not specify
     * maxresults, or specifies a value greater than 5,000, the server will return up to 5,000
     * items.
     */
    Azure::Core::Nullable<int32_t> MaxResults;

    /**
     * @brief Include this parameter to specify one or more datasets to include in the response.
     */
    Azure::Core::Nullable<ListSharesIncludeType> ListSharesInclude;
  };

}}}} // namespace Azure::Storage::Files::Shares
