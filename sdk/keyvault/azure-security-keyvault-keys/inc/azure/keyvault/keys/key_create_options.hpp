// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_operation.hpp"

#include <list>
#include <string>
#include <unordered_map>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  struct CreateKeyOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    std::list<KeyOperation> KeyOperations;

    Azure::Core::Nullable<Azure::Core::DateTime> NotBefore;

    Azure::Core::Nullable<Azure::Core::DateTime> ExpiresOn;

    Azure::Core::Nullable<bool> Enabled;

    std::unordered_map<std::string, std::string> Tags;
  };

}}}} // namespace Azure::Security::KeyVault::Keys
