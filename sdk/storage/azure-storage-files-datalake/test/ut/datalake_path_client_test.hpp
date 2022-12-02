// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/datalake.hpp>

#include "datalake_file_system_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakePathClientTest : public DataLakeFileSystemClientTest {
  protected:
    void SetUp();
    void TearDown();

    std::vector<Files::DataLake::Models::Acl> GetValidAcls();

    std::shared_ptr<Files::DataLake::DataLakePathClient> m_pathClient;
    std::string m_pathName;
  };

}}} // namespace Azure::Storage::Test