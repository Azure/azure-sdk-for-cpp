// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/datalake.hpp>

#include "datalake_path_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeDirectoryClientTest : public DataLakePathClientTest {
  protected:
    void SetUp();
    void TearDown();

    std::shared_ptr<Files::DataLake::DataLakeDirectoryClient> m_directoryClient;
    std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
