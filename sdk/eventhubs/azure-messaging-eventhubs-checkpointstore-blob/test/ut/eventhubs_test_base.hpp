// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/test/test_base.hpp>

#include <gtest/gtest.h>

enum class AuthType
{
  ManagedIdentity,
};

class EventHubsTestBase : public Azure::Core::Test::TestBase,
                          public ::testing::WithParamInterface<AuthType> {
public:
  EventHubsTestBase() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }
  // Create
  virtual void SetUp() override
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
  }
  virtual void TearDown() override
  {
    // Make sure you call the base classes TearDown method to ensure recordings are made.
    TestBase::TearDown();
  }

protected:
};
