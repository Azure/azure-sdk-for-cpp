// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/program.hpp>

#include "azure/performance-stress/test/no_op_test.hpp"

#include <functional>
#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
  // Create the test list
  std::vector<std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
      Azure::PerformanceStress::Options)>>
      tests{
          [](Azure::PerformanceStress::Options opts) {
            // No Op
            return std::make_unique<Azure::PerformanceStress::Test::NoOp>(opts);
          },
          [](Azure::PerformanceStress::Options opts) {
            // Another test
            return std::make_unique<Azure::PerformanceStress::Test::NoOp>(opts);
          }};

  Azure::PerformanceStress::Program::Run(tests, argc, argv);

  return 0;
}
