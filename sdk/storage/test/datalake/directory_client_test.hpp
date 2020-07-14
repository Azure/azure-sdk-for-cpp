// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake.hpp"
#include "file_system_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DirectoryClientTest : public FileSystemClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<Files::DataLake::DirectoryClient> m_directoryClient;
    static std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
