// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares.hpp>

#include "share_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareDirectoryClientTest : public FileShareClientTest {
  protected:
    void SetUp();
    void TearDown();

    std::pair<
        std::vector<Files::Shares::Models::FileItem>,
        std::vector<Files::Shares::Models::DirectoryItem>>
    ListAllFilesAndDirectories(
        const std::string& directoryPath = std::string(),
        const std::string& prefix = std::string());

    std::shared_ptr<Files::Shares::ShareDirectoryClient> m_fileShareDirectoryClient;
    std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
