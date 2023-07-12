// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/files/datalake.hpp>

namespace Azure { namespace Storage { namespace Test {

  class DataLakeServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp() override
    {
      StorageTest::SetUp();
      auto options = InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>();
      m_dataLakeServiceClient = std::make_shared<Files::DataLake::DataLakeServiceClient>(
          Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), options));
    }

  protected:
    std::shared_ptr<Files::DataLake::DataLakeServiceClient> m_dataLakeServiceClient;
  };

}}} // namespace Azure::Storage::Test
