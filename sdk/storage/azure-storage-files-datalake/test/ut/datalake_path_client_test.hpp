// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "datalake_file_system_client_test.hpp"

#include <azure/storage/files/datalake.hpp>

namespace Azure { namespace Storage { namespace Test {

  class DataLakePathClientTest : public DataLakeFileSystemClientTest {
  protected:
    void SetUp() override;

    std::vector<Files::DataLake::Models::Acl> GetAclsForTesting();

    std::string GetDataLakePathUrl(const std::string& fileSystemName, const std::string& path)
    {
      return GetDataLakeFileSystemUrl(fileSystemName) + "/" + path;
    }

  protected:
    std::shared_ptr<Files::DataLake::DataLakePathClient> m_pathClient;
    std::string m_pathName;
  };

}}} // namespace Azure::Storage::Test
