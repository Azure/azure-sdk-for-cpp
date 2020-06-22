// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileSystemClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::vector<DataLake::Path> ListAllPaths(
        bool recursive,
        const std::string& directory = std::string());

    static DataLake::DataLakeHttpHeaders GetInterestingHttpHeaders();

    static std::shared_ptr<DataLake::FileSystemClient> m_fileSystemClient;
    static std::string m_fileSystemName;

    // Path related
    static std::vector<std::string> m_PathNameSetA;
    static std::string m_DirectoryA;
    static std::vector<std::string> m_PathNameSetB;
    static std::string m_DirectoryB;
  };

}}} // namespace Azure::Storage::Test
