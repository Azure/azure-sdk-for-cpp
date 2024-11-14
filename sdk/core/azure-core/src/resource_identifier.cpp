// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/resource_identifier.hpp"

#include <stdexcept>

namespace {
const std::string SubscriptionStart = "/subscriptions/";
const std::string ProviderStart = "/providers/";
} // namespace

namespace Azure { namespace Core {

  ResourceIdentifier::ResourceIdentifier(std::string const& resourceId) : m_resourceId(resourceId)
  {
    // Validate prefix
    if (resourceId.find(SubscriptionStart) != 0 && resourceId.find(ProviderStart) != 0)
    {
      throw std::invalid_argument(
          "The ResourceIdentifier must start with '" + SubscriptionStart + "' or '" + ProviderStart
          + "'.");
    }
  }
}} // namespace Azure::Core
