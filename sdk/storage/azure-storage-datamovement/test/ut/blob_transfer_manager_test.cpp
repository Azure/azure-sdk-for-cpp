// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_transfer_manager_test.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <azure/storage/datamovement/blob_transfer_manager.hpp>
#include <azure/storage/datamovement/directory_iterator.hpp>

namespace Azure { namespace Storage { namespace Test {

  void BlobTransferManagerTest::CreateDir(const std::string& dir)
  {
#if defined(AZ_PLATFORM_WINDOWS)
    CreateDirectory(dir.data(), NULL);
#else
    mkdir(dir.data(), 0777);
#endif
  }

  void BlobTransferManagerTest::DeleteDir(const std::string& dir)
  {
    _internal::DirectoryIterator iterator(dir);
    while (true)
    {
      auto i = iterator.Next();
      if (i.Name.empty())
      {
        break;
      }
      if (i.IsDirectory)
      {
        DeleteDir(dir + "/" + i.Name);
      }
#undef DeleteFile
      DeleteFile(i.Name);
    }
  }

}}} // namespace Azure::Storage::Test
