// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "share_directory_client_test.hpp"
#include "test/ut/test_base.hpp"

#include <azure/storage/files/shares.hpp>

namespace Azure { namespace Storage { namespace Test {

  class FileShareFileClientTest : public FileShareDirectoryClientTest {
  protected:
    void SetUp();

    std::shared_ptr<Files::Shares::ShareFileClient> m_fileClient;
    std::string m_fileName;
  };

}}} // namespace Azure::Storage::Test
