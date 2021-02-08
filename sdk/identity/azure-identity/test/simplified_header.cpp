// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief makes sure azure/core.hpp can be included.
 *
 * @remark This file will catch any issue while trying to use/include the core.hpp header
 *
 */

#include <azure/identity.hpp>
#include <gtest/gtest.h>

class DllExportTest {
  AZ_IDENTITY_DLLEXPORT static const bool DllExportHIncluded;
};

TEST(SimplifiedHeader, identity)
{
  EXPECT_NO_THROW(Azure::Identity::ClientSecretCredential clientSecretCredential("", "", ""));
  EXPECT_NO_THROW(Azure::Identity::EnvironmentCredential environmentCredential);
  EXPECT_NO_THROW(Azure::Identity::Details::Version::VersionString());
}
