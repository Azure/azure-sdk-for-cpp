// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
