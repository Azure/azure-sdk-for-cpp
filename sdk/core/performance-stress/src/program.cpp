// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/program.hpp"
#include "azure/performance-stress/argagg.hpp"

#include <azure/core/internal/json.hpp>
#include <azure/core/internal/strings.hpp>

#include <iostream>

namespace {

std::unique_ptr<Azure::PerformanceStress::PerformanceTest> PrintAvailableTests(
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::TestOptions)>> const& tests)
{
  std::cout << "Available tests to run:" << std::endl;
  for (auto test : tests)
  {
    std::cout << "  - " << test.first << std::endl;
  }
  return nullptr;
}

std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
    Azure::PerformanceStress::TestOptions)>
GetTest(
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::TestOptions)>> const& tests,
    int argc,
    char** argv)
{
  if (argc == 1)
  {
    return nullptr;
  }
  argagg::parser argParser;
  auto args = argParser.parse(argc, argv, true);

  auto testName = std::string(args.pos[0]);

  for (auto test : tests)
  {
    if (Azure::Core::Internal::Strings::LocaleInvariantCaseInsensitiveEqual(test.first, testName))
    {
      return test.second;
    }
  }
  return nullptr;
}

std::string ReplaceAll(std::string src, std::string const& findThis, std::string const& replaceWith)
{
  size_t start_pos = 0;
  while ((start_pos = src.find(findThis, start_pos)) != std::string::npos)
  {
    src.replace(start_pos, findThis.length(), replaceWith);
    start_pos += replaceWith.length();
  }
  return src;
}

void PrintOptions(
    Azure::PerformanceStress::GlobalTestOptions const& options,
    std::vector<Azure::PerformanceStress::TestOption> const& testOptions,
    argagg::parser_results const& parsedArgs)
{
  {
    std::cout << std::endl << "=== Global Options ===" << std::endl;
    Azure::Core::Internal::Json::json optionsJs = options;
    std::cout << ReplaceAll(optionsJs.dump(), ",", ",\n") << std::endl;
  }

  if (testOptions.size() > 0)
  {
    std::cout << std::endl << "=== Test Options ===" << std::endl;
    Azure::Core::Internal::Json::json optionsJs;
    for (auto option : testOptions)
    {
      optionsJs[option.Name] = parsedArgs[option.Name].as<std::string>();
    }
    std::cout << ReplaceAll(optionsJs.dump(), ",", ",\n") << std::endl;
  }
}

} // namespace

void Azure::PerformanceStress::Program::Run(
    Azure::Core::Context context,
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::TestOptions)>> const& tests,
    int argc,
    char** argv)
{
  (void)context;

  // Parse args only to get the test name first
  auto testGenerator = GetTest(tests, argc, argv);
  if (testGenerator == nullptr)
  {
    // Wrong input. Print what are the options.
    PrintAvailableTests(tests);
    return;
  }
  // Initial test to get it's options, we can use a dummy parser results
  argagg::parser_results argResults;
  auto test = testGenerator(Azure::PerformanceStress::TestOptions(argResults));
  auto testOptions = test->GetTestOptions();
  argResults = Azure::PerformanceStress::Program::ArgParser::Parse(argc, argv, testOptions);
  // ReCreate Test with parsed results
  test = testGenerator(Azure::PerformanceStress::TestOptions(argResults));
  auto options = Azure::PerformanceStress::Program::ArgParser::Parse(argResults);

  // Print options
  PrintOptions(options, testOptions, argResults);

  // auto testOptions = test->GetTestOptions();
  // if (testOptions.size() > 0)
  //   std::cout << "=== Global Options ===" << std::endl;

  // test->GlobalSetup();
  test->Run(context);
  // test->GlobalCleanup();
  // (void)tests;
}
