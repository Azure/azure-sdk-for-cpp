// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeServiceClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::vector<Files::DataLake::FileSystem> ListAllFileSystems(
        const std::string& prefix = std::string());

    static std::shared_ptr<Files::DataLake::ServiceClient> m_dataLakeServiceClient;
    static std::vector<std::string> m_fileSystemNameSetA;
    static std::string m_fileSystemPrefixA;
    static std::vector<std::string> m_fileSystemNameSetB;
    static std::string m_fileSystemPrefixB;
  };

}}} // namespace Azure::Storage::Test
