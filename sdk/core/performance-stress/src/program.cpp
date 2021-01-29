// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/program.hpp"

#include <azure/core/internal/json.hpp>
#include <azure/core/internal/strings.hpp>

#include <iostream>

namespace {
std::function<
    std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(Azure::PerformanceStress::Options)>
GetTest(
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::Options)>> const& tests,
    std::string const& testName)
{
  for (auto test : tests)
  {
    if (Azure::Core::Internal::Strings::LocaleInvariantCaseInsensitiveEqual(test.first, testName))
    {
      return test.second;
    }
  }
  throw std::runtime_error("No test name with name: " + testName);
}
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
{
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
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
  (void)context;
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

  // Get the test generator to run
  auto testGenerator = GetTest(tests, testName);

  // Print options
  std::cout << "=== Options ===" << std::endl;
  Azure::Core::Internal::Json::json optionsJs = options;
  std::cout << ReplaceAll(optionsJs.dump(), ",", ",\n") << std::endl;

  // test->GlobalSetup();
  // test->Run(context);
  // test->GlobalCleanup();
  // (void)tests;
}
