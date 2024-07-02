// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/test/eventhubs_batch_perf_test.hpp"

#include <azure/perf.hpp>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Messaging::EventHubs::PerfTest::Batch::BatchTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context{}, tests, argc, argv);

  return 0;
}
