// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "datalake_file_system_client_test.hpp"

#include <azure/storage/files/datalake.hpp>

namespace Azure { namespace Storage { namespace Test {

  class DataLakeFileClientTest : public DataLakeFileSystemClientTest {
  protected:
    void SetUp() override;

    std::string GetDataLakeFileUrl(const std::string& fileSystemName, const std::string& filePath)
    {
      return GetDataLakeFileSystemUrl(fileSystemName) + "/" + filePath;
    }

  protected:
    std::shared_ptr<Files::DataLake::DataLakeFileClient> m_fileClient;
    std::string m_fileName;
  };

}}} // namespace Azure::Storage::Test
