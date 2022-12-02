// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp();
    void TearDown();

    Files::Shares::Models::FileHttpHeaders GetInterestingHttpHeaders();

    std::shared_ptr<Files::Shares::ShareClient> m_shareClient;
    std::string m_shareName;
    Files::Shares::ShareClientOptions m_options;
    std::string m_testName;
    std::string m_testNameLowercase;
  };

}}} // namespace Azure::Storage::Test