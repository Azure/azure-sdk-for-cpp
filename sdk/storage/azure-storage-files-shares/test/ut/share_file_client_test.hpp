// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares.hpp>

#include "share_directory_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareFileClientTest : public FileShareDirectoryClientTest {
  protected:
    void SetUp();
    void TearDown();

    std::shared_ptr<Files::Shares::ShareFileClient> m_fileClient;
    std::string m_fileName;
    std::vector<uint8_t> m_fileContent;
  };

}}} // namespace Azure::Storage::Test