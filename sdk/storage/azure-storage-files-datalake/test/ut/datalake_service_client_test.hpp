// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/datalake.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class DataLakeServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    std::shared_ptr<Files::DataLake::DataLakeServiceClient> m_dataLakeServiceClient;
    std::vector<std::string> m_fileSystemNameSetA;
    std::string m_fileSystemPrefixA;
    std::vector<std::string> m_fileSystemNameSetB;
    std::string m_fileSystemPrefixB;

    virtual void SetUp() override;

    void CreateFileSystemList();

    virtual void TearDown() override
    {
      for (const auto& name : m_fileSystemNameSetA)
      {
        m_dataLakeServiceClient->GetFileSystemClient(name).Delete();
      }
      for (const auto& name : m_fileSystemNameSetB)
      {
        m_dataLakeServiceClient->GetFileSystemClient(name).Delete();
      }

      StorageTest::TearDown();
    }

    std::vector<Files::DataLake::Models::FileSystemItem> ListAllFileSystems(
        const std::string& prefix = std::string());
  };

}}} // namespace Azure::Storage::Test
