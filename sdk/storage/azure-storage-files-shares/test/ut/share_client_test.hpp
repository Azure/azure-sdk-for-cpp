// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_service_client_test.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareClientTest : public FileShareServiceClientTest {
  protected:
    void SetUp() override;

    Files::Shares::ShareClient GetShareClientForTest(
        const std::string& shareName,
        Files::Shares::ShareClientOptions clientOptions = Files::Shares::ShareClientOptions());

    std::string GetShareUrl(const std::string& shareName)
    {
      return GetShareServiceUrl() + "/" + shareName;
    }

  protected:
    std::shared_ptr<Files::Shares::ShareClient> m_shareClient;
    std::string m_shareName;
  };

}}} // namespace Azure::Storage::Test
