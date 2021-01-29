// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/program.hpp"

void Azure::PerformanceStress::Program::Run(
    std::vector<std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
        Azure::PerformanceStress::Options)>> const& tests,
    int argc,
    char** argv)
{
  Azure::PerformanceStress::Options options;
  try
  {
    options = Azure::PerformanceStress::Program::ArgParser::Parse(argc, argv);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unable to parse input parameters." << '\n';
    std::abort();
  }
  (void)tests;
}
