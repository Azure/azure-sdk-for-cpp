// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares.hpp>

#include "share_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareDirectoryClientTest : public FileShareClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::pair<
        std::vector<Files::Shares::Models::FileItem>,
        std::vector<Files::Shares::Models::DirectoryItem>>
    ListAllFilesAndDirectories(
        const std::string& directoryPath = std::string(),
        const std::string& prefix = std::string());

    static Files::Shares::Models::FileHttpHeaders GetInterestingHttpHeaders();

    static std::shared_ptr<Files::Shares::ShareDirectoryClient> m_fileShareDirectoryClient;
    static std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
