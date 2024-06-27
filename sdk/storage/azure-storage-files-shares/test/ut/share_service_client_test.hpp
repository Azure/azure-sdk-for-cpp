// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp() override;

    Files::Shares::ShareClient GetPremiumShareClientForTest(
        const std::string& shareName,
        Files::Shares::ShareClientOptions clientOptions = Files::Shares::ShareClientOptions());

    std::string GetShareServiceUrl()
    {
      return "https://" + StandardStorageAccountName() + ".file.core.windows.net";
    }

    std::string GetPremiumShareServiceUrl()
    {
      return "https://" + PremiumFileAccountName() + ".file.core.windows.net";
    }

  protected:
    std::shared_ptr<Files::Shares::ShareServiceClient> m_shareServiceClient;
    std::shared_ptr<Files::Shares::ShareServiceClient> m_premiumShareServiceClient;
  };

}}} // namespace Azure::Storage::Test
