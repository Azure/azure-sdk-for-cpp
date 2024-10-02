// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/resource_identifier.hpp"

#include <stdexcept>
#include <string>

namespace Azure { namespace Core {

  ResourceIdentifier::ResourceIdentifier(std::string const& resourceId) : m_resourceId(resourceId)
  {
    const std::string subscriptionStart = "/subscriptions/";
    const std::string providerStart = "/providers/";

    // Validate prefix
    if (resourceId.find(subscriptionStart) != 0 && resourceId.find(providerStart) != 0)
    {
      throw std::invalid_argument(
          "The ResourceIdentifier must start with " + subscriptionStart + " or " + providerStart
          + ".");
    }
  }
}} // namespace Azure::Core
