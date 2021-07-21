// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Keys Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {

	/***************** KeyVault Secret *****************/
  constexpr static const char SecretPath[] = "secrets";

  /******************* Secret property names ***********/

  constexpr static const char AttributesPropertyName[] = "attributes";
  constexpr static const char EnabledPropertyName[] = "enabled";
  constexpr static const char NbfPropertyName[] = "nbf";
  constexpr static const char ExpPropertyName[] = "exp";
  constexpr static const char CreatedPropertyName[] = "created";
  constexpr static const char UpdatedPropertyName[] = "updated";
  constexpr static const char ManagedPropertyName[] = "managed";
  constexpr static const char TagsPropertyName[] = "tags";
  constexpr static const char IdPropertyName[] = "id";
  constexpr static const char KeyIdPropertyName[] = "kid";
  constexpr static const char ValuePropertyName[] = "value";
  constexpr static const char RecoveryLevelPropertyName[] = "recoveryLevel";
  constexpr static const char ContentTypePropertyName[] = "contentType";
  constexpr static const char RecoverableDaysPropertyName[] = "recoverableDays";

    /**************** KeyVault QueryParameters *********/
  static constexpr char const ApiVersion[] = "api-version";
}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail
