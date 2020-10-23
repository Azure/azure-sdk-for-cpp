// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake.hpp"
#include "datalake_file_system_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakePathClientTest : public DataLakeFileSystemClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::vector<Files::DataLake::Acl> GetValidAcls();

    static std::shared_ptr<Files::DataLake::PathClient> m_pathClient;
    static std::string m_pathName;
  };

}}} // namespace Azure::Storage::Test
