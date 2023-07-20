// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "gtest/gtest.h"

#include <azure/core/test/test_base.hpp>

class EventHubsTestBase : public Azure::Core::Test::TestBase {
  // Create
  virtual void SetUp() override
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
  }
};
