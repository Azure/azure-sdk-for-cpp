// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_client_test.hpp"
#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareDirectoryClientTest : public FileShareClientTest {
  protected:
    void SetUp() override;

    std::shared_ptr<Files::Shares::ShareDirectoryClient> m_fileShareDirectoryClient;
    std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
