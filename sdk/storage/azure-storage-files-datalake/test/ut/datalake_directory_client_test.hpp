// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "datalake_path_client_test.hpp"

#include <azure/storage/files/datalake.hpp>

namespace Azure { namespace Storage { namespace Test {

  class DataLakeDirectoryClientTest : public DataLakePathClientTest {
  protected:
    void SetUp() override;

  protected:
    std::shared_ptr<Files::DataLake::DataLakeDirectoryClient> m_directoryClient;
    std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
