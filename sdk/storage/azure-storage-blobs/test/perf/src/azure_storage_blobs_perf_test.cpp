// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/storage/blobs/test/download_blob_from_sas.hpp"
#include "azure/storage/blobs/test/download_blob_pipeline_only.hpp"
#include "azure/storage/blobs/test/download_blob_test.hpp"

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/storage/blobs/test/download_blob_transport_only.hpp"
#endif

#include "azure/storage/blobs/test/list_blob_test.hpp"
#include "azure/storage/blobs/test/upload_blob_test.hpp"

int main(int argc, char** argv)
{
  std::cout << "Azure-storage-blobs VERSION " << VCPKG_STORAGE_BLOB_VERSION << std::endl;

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests
  {
    Azure::Storage::Blobs::Test::DownloadBlob::GetTestMetadata(),
        Azure::Storage::Blobs::Test::UploadBlob::GetTestMetadata(),
        Azure::Storage::Blobs::Test::ListBlob::GetTestMetadata(),
        Azure::Storage::Blobs::Test::DownloadBlobSas::GetTestMetadata(),
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
        Azure::Storage::Blobs::Test::DownloadBlobWithTransportOnly::GetTestMetadata(),
#endif
        Azure::Storage::Blobs::Test::DownloadBlobWithPipelineOnly::GetTestMetadata()
  };

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}