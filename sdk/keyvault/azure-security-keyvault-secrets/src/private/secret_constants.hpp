// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Secret Client.
 *
 */

#pragma once

#include <cstddef>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {

  constexpr static const char KeyVaultServicePackageName[] = "keyvault-secrets";
  /***************** KeyVault Secret *****************/
  constexpr static const char SecretPath[] = "secrets";
  static constexpr char const DeletedSecretPath[] = "deletedsecrets";
  static constexpr char const BackupSecretPath[] = "backup";
  static constexpr char const RestoreSecretPath[] = "restore";
  static constexpr char const RecoverDeletedSecretPath[] = "recover";

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

  /**************** Deleted Secret property names ********/
  constexpr static const char RecoveryIdPropertyName[] = "recoveryId";
  constexpr static const char ScheduledPurgeDatePropertyName[] = "scheduledPurgeDate";
  constexpr static const char DeletedDatePropertyName[] = "deletedDate";

  /**************** KeyVault QueryParameters *********/
  static constexpr char const ApiVersion[] = "api-version";

  /**************** KeyVault Secrets Paged  *********/
  static constexpr size_t PagedMaxResults = 25;
  static constexpr char const PagedMaxResultsName[] = "maxresults";
  static constexpr char const VersionsName[] = "versions";

}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail
