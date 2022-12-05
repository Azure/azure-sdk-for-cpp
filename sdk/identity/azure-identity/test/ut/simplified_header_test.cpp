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

class DllExportTest final {
  AZ_IDENTITY_DLLEXPORT static const bool DllExportHIncluded;
};

TEST(SimplifiedHeader, identity)
{
  using namespace Azure::Identity;

  EXPECT_NO_THROW(ClientSecretCredential clientSecretCredential("", "", ""));
  EXPECT_NO_THROW(EnvironmentCredential environmentCredential);
  EXPECT_NO_THROW(static_cast<void>(static_cast<ManagedIdentityCredential const*>(nullptr)));
}