// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/storage/blobs/test/download_blob_from_sas.hpp"
#include "azure/storage/blobs/test/download_blob_pipeline_only.hpp"
#include "azure/storage/blobs/test/download_blob_test.hpp"
#include "azure/storage/blobs/test/download_blob_trasport_only.hpp"
#include "azure/storage/blobs/test/list_blob_test.hpp"
#include "azure/storage/blobs/test/upload_blob_test.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Storage::Blobs::Test::DownloadBlob::GetTestMetadata(),
      Azure::Storage::Blobs::Test::UploadBlob::GetTestMetadata(),
      Azure::Storage::Blobs::Test::ListBlob::GetTestMetadata(),
      Azure::Storage::Blobs::Test::DownloadBlobSas::GetTestMetadata(),
      Azure::Storage::Blobs::Test::DownloadBlobWithTransportOnly::GetTestMetadata(),
      Azure::Storage::Blobs::Test::DownloadBlobWithPipelineOnly::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}
