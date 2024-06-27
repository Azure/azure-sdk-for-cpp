// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_directory_client_test.hpp"
#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareFileClientTest : public FileShareDirectoryClientTest {
  protected:
    void SetUp();

    std::string GetShareFileUrl(const std::string& shareName, const std::string& FileName)
    {
      return GetShareUrl(shareName) + "/" + FileName;
    }

    std::shared_ptr<Files::Shares::ShareFileClient> m_fileClient;
    std::string m_fileName;
  };

}}} // namespace Azure::Storage::Test
