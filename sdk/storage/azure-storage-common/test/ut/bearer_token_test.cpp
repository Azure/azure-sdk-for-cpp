//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(ClientSecretCredentialTest, ClientSecretCredentialWorks)
  {
    auto containerClient = GetClientForTest(GetTestName());

    EXPECT_NO_THROW(containerClient.Create());
    EXPECT_NO_THROW(containerClient.Delete());
  }

}}} // namespace Azure::Storage::Test