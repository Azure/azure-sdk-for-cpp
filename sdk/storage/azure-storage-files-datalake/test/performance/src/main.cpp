// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance_framework.hpp>

#include "azure/storage/files/datalake/test/performance/download_file.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::PerformanceStress::TestMetadata> tests{
      Azure::Storage::Files::DataLake::Test::Performance::DownloadFile::GetTestMetadata()};

  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
