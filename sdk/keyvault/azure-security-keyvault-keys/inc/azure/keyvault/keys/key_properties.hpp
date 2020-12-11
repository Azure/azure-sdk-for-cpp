// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_release_policy.hpp"

#include <string>
#include <unordered_map>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  namespace Details {
    constexpr static const char* ManagedPropertyName = "managed";
    constexpr static const char* AttributesPropertyName = "attributes";
    constexpr static const char* TagsPropertyName = "tags";
    constexpr static const char* ReleasePolicyPropertyName = "release_policy";
  } // namespace Details

  struct KeyProperties
  {
    std::string Name;
    std::string Id;
    std::string VaultUrl;
    std::string Version;
    bool Managed;
    std::unordered_map<std::string, std::string> Tags;
    Azure::Core::Nullable<bool> Enabled;
    Azure::Core::Nullable<Azure::Core::DateTime> NotBefore;
    Azure::Core::Nullable<Azure::Core::DateTime> ExpiresOn;
    Azure::Core::Nullable<Azure::Core::DateTime> CreatedOn;
    Azure::Core::Nullable<Azure::Core::DateTime> UpdatedOn;
    Azure::Core::Nullable<int> RecoverableDays;
    std::string RecoveryLevel;
    Azure::Core::Nullable<bool> Exportable;
    KeyReleasePolicy ReleasePolicy;

    KeyProperties() {}
    KeyProperties(std::string const& name) : Name(name) {}
  };

}}}} // namespace Azure::Security::KeyVault::Keys
