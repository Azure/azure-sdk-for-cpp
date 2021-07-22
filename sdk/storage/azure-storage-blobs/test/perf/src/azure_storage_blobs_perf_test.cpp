// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/storage/blobs/test/download_blob_test.hpp"
#include "azure/storage/blobs/test/list_blob_test.hpp"
#include "azure/storage/blobs/test/upload_blob_test.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Storage::Blobs::Test::DownloadBlob::GetTestMetadata(),
      Azure::Storage::Blobs::Test::UploadBlob::GetTestMetadata(),
      Azure::Storage::Blobs::Test::ListBlob::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}
