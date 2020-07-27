// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "common/constants.hpp"
#include "common/storage_credential.hpp"

namespace Azure { namespace Storage {

  enum class SasProtocol
  {
    HttpsAndHtttp,
    HttpsOnly,
  };

  enum class AccountSasResource
  {
    None = 0,
    Service = 1,
    Container = 2,
    Object = 4,
    All = ~0,
  };

  inline AccountSasResource operator|(AccountSasResource lhs, AccountSasResource rhs)
  {
    using type = std::underlying_type_t<AccountSasResource>;
    return static_cast<AccountSasResource>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline AccountSasResource operator&(AccountSasResource lhs, AccountSasResource rhs)
  {
    using type = std::underlying_type_t<AccountSasResource>;
    return static_cast<AccountSasResource>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  enum class AccountSasServices
  {
    None = 0,
    Blobs = 1,
    Queue = 2,
    Files = 4,
    All = ~0,
  };

  inline AccountSasServices operator|(AccountSasServices lhs, AccountSasServices rhs)
  {
    using type = std::underlying_type_t<AccountSasServices>;
    return static_cast<AccountSasServices>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline AccountSasServices operator&(AccountSasServices lhs, AccountSasServices rhs)
  {
    using type = std::underlying_type_t<AccountSasServices>;
    return static_cast<AccountSasServices>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  enum class AccountSasPermissions
  {
    None = 0,
    Read = 1,
    Write = 2,
    Delete = 4,
    DeleteVersion = 8,
    List = 16,
    Add = 32,
    Create = 64,
    Update = 128,
    Process = 256,
    Tags = 512,
    Filter = 1024,
    All = ~0,
  };

  inline AccountSasPermissions operator|(AccountSasPermissions lhs, AccountSasPermissions rhs)
  {
    using type = std::underlying_type_t<AccountSasPermissions>;
    return static_cast<AccountSasPermissions>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  inline AccountSasPermissions operator&(AccountSasPermissions lhs, AccountSasPermissions rhs)
  {
    using type = std::underlying_type_t<AccountSasPermissions>;
    return static_cast<AccountSasPermissions>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  struct AccountSasBuilder
  {
    std::string Version = Details::c_defaultSasVersion;
    SasProtocol Protocol;
    std::string StartsOn;
    std::string ExpiresOn;
    std::string IPRange;
    AccountSasServices Services;
    AccountSasResource ResourceTypes;

    void SetPermissions(AccountSasPermissions permissions);

    std::string ToSasQueryParameters(const SharedKeyCredential& credential);

  private:
    std::string Permissions;
  };

}} // namespace Azure::Storage
