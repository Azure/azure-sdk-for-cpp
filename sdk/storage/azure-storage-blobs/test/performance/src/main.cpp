// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance_framework.hpp>

#include "azure/storage/blobs/test/performance/download_blob.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::PerformanceStress::TestMetadata> tests{
      Azure::Storage::Blobs::Test::Performance::DownloadBlob::GetTestMetadata()};

  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
