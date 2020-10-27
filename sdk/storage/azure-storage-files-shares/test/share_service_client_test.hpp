// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareServiceClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::vector<Files::Shares::ShareItem> ListAllShares(
        const std::string& prefix = std::string());

    static std::shared_ptr<Files::Shares::ServiceClient> m_fileShareServiceClient;
    static std::vector<std::string> m_shareNameSetA;
    static std::string m_sharePrefixA;
    static std::vector<std::string> m_shareNameSetB;
    static std::string m_sharePrefixB;
  };

}}} // namespace Azure::Storage::Test
