// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_client_test.hpp"
#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareDirectoryClientTest : public FileShareClientTest {
  protected:
    void SetUp() override;

    std::string GetShareDirectoryUrl(const std::string& shareName, const std::string& directoryName)
    {
      return GetShareUrl(shareName) + "/" + directoryName;
    }

    std::shared_ptr<Files::Shares::ShareDirectoryClient> m_fileShareDirectoryClient;
    std::string m_directoryName;
  };

}}} // namespace Azure::Storage::Test
