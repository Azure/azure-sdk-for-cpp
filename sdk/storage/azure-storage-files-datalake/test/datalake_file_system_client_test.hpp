// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeFileSystemClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::vector<Files::DataLake::Path> ListAllPaths(
        bool recursive,
        const std::string& directory = std::string());

    static Files::DataLake::DataLakeHttpHeaders GetInterestingHttpHeaders();

    static std::shared_ptr<Files::DataLake::FileSystemClient> m_fileSystemClient;
    static std::string m_fileSystemName;

    // Path related
    static std::vector<std::string> m_pathNameSetA;
    static std::string m_directoryA;
    static std::vector<std::string> m_pathNameSetB;
    static std::string m_directoryB;
  };

}}} // namespace Azure::Storage::Test
