//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/datalake.hpp>

#include "datalake_file_system_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeFileClientTest : public DataLakeFileSystemClientTest {
  protected:
    void SetUp();
    void TearDown();

    std::shared_ptr<Files::DataLake::DataLakeFileClient> m_fileClient;
    std::string m_fileName;
  };

}}} // namespace Azure::Storage::Test
