// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares.hpp>

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_POSIX)
#include <unistd.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "share_directory_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class FileShareFileClientTest : public FileShareDirectoryClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<Files::Shares::ShareFileClient> m_fileClient;
    static std::string m_fileName;
    static std::vector<uint8_t> m_fileContent;
  };

}}} // namespace Azure::Storage::Test
