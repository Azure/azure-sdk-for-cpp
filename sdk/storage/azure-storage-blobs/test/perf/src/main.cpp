// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/storage/blobs/test/perf/download_blob.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Storage::Blobs::Test::Performance::DownloadBlob::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
