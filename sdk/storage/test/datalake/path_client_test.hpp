// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake.hpp"
#include "file_system_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class PathClientTest : public FileSystemClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<DataLake::PathClient> m_pathClient;
    static std::string m_pathName;
  };

}}} // namespace Azure::Storage::Test
