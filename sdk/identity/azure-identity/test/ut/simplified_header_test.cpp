// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief makes sure azure/identity.hpp can be included.
 *
 * @remark This file will catch any issue while trying to use/include the identity.hpp header
 *
 */

#include <azure/identity.hpp>

#include <gtest/gtest.h>

TEST(SimplifiedHeader, identity)
{
  using namespace Azure::Identity;

  static_assert(
      sizeof(AzureCliCredential) != 0,
      "azure/identity.hpp does not include azure_cli_credential.hpp");

  static_assert(
      sizeof(ChainedTokenCredential) != 0,
      "azure/identity.hpp does not include chained_token_credential.hpp");

  static_assert(
      sizeof(ClientCertificateCredential) != 0,
      "azure/identity.hpp does not include client_certificate_credential.hpp");

  static_assert(
      sizeof(ClientSecretCredential) != 0,
      "azure/identity.hpp does not include client_secret_credential.hpp");

  static_assert(
      sizeof(DefaultAzureCredential) != 0,
      "azure/identity.hpp does not include default_azure_credential.hpp");

#if !defined(AZ_IDENTITY_DLLEXPORT)
  static_assert(false, "azure/identity.hpp does not include dll_import_export.hpp");
#endif

  static_assert(
      sizeof(EnvironmentCredential) != 0,
      "azure/identity.hpp does not include environment_credential.hpp");

  static_assert(
      sizeof(ManagedIdentityCredential) != 0,
      "azure/identity.hpp does not include managed_identity_credential.hpp");

#if !defined(AZ_IDENTITY_RTTI)
  static_assert(false, "azure/identity.hpp does not include rtti.hpp");
#endif
}
