// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake.hpp"
#include "datalake_file_system_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeFileClientTest : public DataLakeFileSystemClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<Files::DataLake::FileClient> m_fileClient;
    static std::string m_fileName;
  };

}}} // namespace Azure::Storage::Test
