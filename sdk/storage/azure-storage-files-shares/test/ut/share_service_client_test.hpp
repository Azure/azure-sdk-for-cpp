// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp() override;

  protected:
    std::shared_ptr<Files::Shares::ShareServiceClient> m_shareServiceClient;
  };

}}} // namespace Azure::Storage::Test
