// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "test/ut/test_base.hpp"

#include <azure/storage/files/datalake.hpp>

namespace Azure { namespace Storage { namespace Test {

  class DataLakeServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp() override
    {
      StorageTest::SetUp();
      auto options = InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>();
      const auto serviceUrl = GetDataLakeServiceUrl();
      if (m_useTokenCredentialByDefault)
      {
        m_dataLakeServiceClient = std::make_shared<Files::DataLake::DataLakeServiceClient>(
            Files::DataLake::DataLakeServiceClient(serviceUrl, GetTestCredential(), options));
      }
      else
      {
        m_dataLakeServiceClient = std::make_shared<Files::DataLake::DataLakeServiceClient>(
            Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
                AdlsGen2ConnectionString(), options));
      }
    }

    Files::DataLake::DataLakeServiceClient GetDataLakeServiceClientOAuth()
    {
      if (m_useTokenCredentialByDefault)
      {
        return *m_dataLakeServiceClient;
      }
      else
      {
        auto options = InitStorageClientOptions<Files::DataLake::DataLakeClientOptions>();
        return Files::DataLake::DataLakeServiceClient(
            m_dataLakeServiceClient->GetUrl(), GetTestCredential(), options);
      }
    }

    std::string GetDataLakeServiceUrl()
    {
      return "https://" + AdlsGen2AccountName() + ".dfs.core.windows.net";
    }

  protected:
    std::shared_ptr<Files::DataLake::DataLakeServiceClient> m_dataLakeServiceClient;
  };

}}} // namespace Azure::Storage::Test
