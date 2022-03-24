// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/datalake.hpp>

#include "datalake_service_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeFileSystemClientTest : public DataLakeServiceClientTest {
  protected:
    void SetUp();
    void TearDown();
    std::string GetSas();
    void CreateDirectoryList();

    std::vector<Files::DataLake::Models::PathItem> ListAllPaths(
        bool recursive,
        const std::string& directory = std::string());

    Files::DataLake::Models::PathHttpHeaders GetInterestingHttpHeaders();

    std::shared_ptr<Files::DataLake::DataLakeFileSystemClient> m_fileSystemClient;
    std::string m_fileSystemName;

    // Path related
    std::vector<std::string> m_pathNameSetA;
    std::string m_directoryA;
    std::vector<std::string> m_pathNameSetB;
    std::string m_directoryB;
  };

}}} // namespace Azure::Storage::Test
