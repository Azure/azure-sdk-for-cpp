// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares.hpp>

#include "share_service_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareClientTest : public FileShareServiceClientTest {
  protected:
    void SetUp() override;

    Files::Shares::ShareClient GetShareClientForTest(
        const std::string& shareName,
        Files::Shares::ShareClientOptions clientOptions = Files::Shares::ShareClientOptions());
    Files::Shares::ShareClient GetPremiumShareClientForTest(
        const std::string& shareName,
        Files::Shares::ShareClientOptions clientOptions = Files::Shares::ShareClientOptions());

  protected:
    std::shared_ptr<Files::Shares::ShareClient> m_shareClient;
    std::string m_shareName;
  };

}}} // namespace Azure::Storage::Test
