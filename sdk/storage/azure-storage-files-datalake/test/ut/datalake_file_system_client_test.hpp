// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "datalake_service_client_test.hpp"

#include <azure/storage/files/datalake.hpp>

namespace Azure { namespace Storage { namespace Test {

  class DataLakeFileSystemClientTest : public DataLakeServiceClientTest {
  protected:
    void SetUp() override;

    std::string GetSas();

    Files::DataLake::DataLakeFileSystemClient GetFileSystemClientForTest(
        const std::string& fileSystemName,
        Files::DataLake::DataLakeClientOptions clientOptions
        = Files::DataLake::DataLakeClientOptions());

    std::string GetDataLakeFileSystemUrl(const std::string& fileSystemName)
    {
      return GetDataLakeServiceUrl() + "/" + fileSystemName;
    }

  protected:
    std::shared_ptr<Files::DataLake::DataLakeFileSystemClient> m_fileSystemClient;
    std::string m_fileSystemName;
  };

}}} // namespace Azure::Storage::Test
