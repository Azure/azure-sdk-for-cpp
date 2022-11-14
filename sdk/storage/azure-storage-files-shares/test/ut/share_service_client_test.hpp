//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp();
    void TearDown();

    std::vector<Files::Shares::Models::ShareItem> ListAllShares(
        const std::string& prefix = std::string());

    std::shared_ptr<Files::Shares::ShareServiceClient> m_fileShareServiceClient;
    std::vector<std::string> m_shareNameSetA;
    std::string m_sharePrefixA;
    std::vector<std::string> m_shareNameSetB;
    std::string m_sharePrefixB;
    Files::Shares::ShareClientOptions m_options;
    std::string m_testName;
    std::string m_testNameLowercase;
  };

}}} // namespace Azure::Storage::Test
