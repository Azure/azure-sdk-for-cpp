// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/program.hpp"

#include <azure/core/internal/strings.hpp>

#include <iostream>

namespace {
std::unique_ptr<Azure::PerformanceStress::PerformanceTest> GetTest(
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::Options)>> const& tests,
    std::string const& testName,
    Azure::PerformanceStress::Options& options)
{
  for (auto test : tests)
  {
    if (Azure::Core::Internal::Strings::LocaleInvariantCaseInsensitiveEqual(test.first, testName))
    {
      return test.second(options);
    }
  }
  throw std::runtime_error("No test name with name: " + testName);
}
} // namespace

void Azure::PerformanceStress::Program::Run(
    Azure::Core::Context context,
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::Options)>> const& tests,
    int argc,
    char** argv)
{
  // Create Options and find out the requested test name
  std::string testName;
  Azure::PerformanceStress::Options options;
  try
  {
    options = Azure::PerformanceStress::Program::ArgParser::Parse(argc, argv, testName);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unable to parse input parameters." << std::endl << e.what() << std::endl;
    std::abort();
  }

  // Get the test to run
  auto test = GetTest(tests, testName, options);
  test->GlobalSetup();
  test->Run(context);
  test->GlobalCleanup();
  (void)tests;
}
