// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/test/http_transport_test.hpp"
#include "azure/core/test/json_test.hpp"
#include "azure/core/test/nullable_test.hpp"
#include "azure/core/test/pipeline_test.hpp"
#include "azure/core/test/uuid_test.hpp"

#include <azure/perf.hpp>

#include <vector>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Core::Test::HTTPTransportTest::GetTestMetadata(),
      Azure::Core::Test::JsonTest::GetTestMetadata(),
      Azure::Core::Test::NullableTest::GetTestMetadata(),
      Azure::Core::Test::PipelineTest::GetTestMetadata(),
      Azure::Core::Test::UuidTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}
