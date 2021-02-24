// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

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

#include "blob_container_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlockBlobClientTest : public BlobContainerClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<Azure::Storage::Blobs::BlockBlobClient> m_blockBlobClient;
    static std::string m_blobName;
    static Azure::Storage::Blobs::UploadBlockBlobOptions m_blobUploadOptions;
    static std::vector<uint8_t> m_blobContent;
  };

}}} // namespace Azure::Storage::Test
