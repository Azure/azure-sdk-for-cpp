// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/test/delay_test.hpp"
#include "azure/core/test/exception_test.hpp"
#include "azure/core/test/extended_options_test.hpp"
#include "azure/core/test/http_transport_test.hpp"
#include "azure/core/test/json_test.hpp"
#include "azure/core/test/no_op_test.hpp"
#include "azure/core/test/nullable_test.hpp"
#include "azure/core/test/pipeline_test.hpp"
#include "azure/core/test/uuid_test.hpp"

#include <azure/perf.hpp>

#include <vector>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Core::Test::DelayTest::GetTestMetadata(),
      Azure::Core::Test::ExceptionTest::GetTestMetadata(),
      Azure::Core::Test::ExtendedOptionsTest::GetTestMetadata(),
      Azure::Core::Test::HTTPTransportTest::GetTestMetadata(),
      Azure::Core::Test::JsonTest::GetTestMetadata(),
      Azure::Core::Test::NoOp::GetTestMetadata(),
      Azure::Core::Test::NullableTest::GetTestMetadata(),
      Azure::Core::Test::PipelineTest::GetTestMetadata(),
      Azure::Core::Test::UuidTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context{}, tests, argc, argv);

  return 0;
}
