// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/shares.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static Files::Shares::FileShareHttpHeaders GetInterestingHttpHeaders();

    static std::shared_ptr<Files::Shares::ShareClient> m_shareClient;
    static std::string m_shareName;
  };

}}} // namespace Azure::Storage::Test
