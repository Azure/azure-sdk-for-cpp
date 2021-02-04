// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/program.hpp"
#include "azure/performance-stress/argagg.hpp"

#include <azure/core/internal/json.hpp>
#include <azure/core/internal/strings.hpp>

#include <chrono>
#include <iostream>
#include <thread>

namespace {

inline std::unique_ptr<Azure::PerformanceStress::PerformanceTest> PrintAvailableTests(
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

inline std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
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

inline std::string ReplaceAll(
    std::string src,
    std::string const& findThis,
    std::string const& replaceWith)
{
  size_t start_pos = 0;
  while ((start_pos = src.find(findThis, start_pos)) != std::string::npos)
  {
    src.replace(start_pos, findThis.length(), replaceWith);
    start_pos += replaceWith.length();
  }
  return src;
}

inline void PrintOptions(
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
      try
      {
        optionsJs[option.Name] = parsedArgs[option.Name].as<std::string>();
      }
      catch (std::out_of_range const& e)
      {
        if (!option.required)
        {
          // arg was not parsed
          optionsJs[option.Name] = "default value";
        }
        else
        {
          // re-throw
          throw std::invalid_argument("Missing mandatory parameter: " + option.Name);
        }
      }
      catch (std::exception const& e)
      {
        throw;
      }
    }
    std::cout << ReplaceAll(optionsJs.dump(), ",", ",\n") << std::endl;
  }
}

inline void RunLoop(
    Azure::Core::Context const& context,
    Azure::PerformanceStress::PerformanceTest& test,
    uint64_t& completedOperations,
    std::chrono::nanoseconds& lastCompletionTimes,
    bool latency,
    bool& isCancelled)
{
  (void)latency;
  auto start = std::chrono::system_clock::now();
  while (!isCancelled)
  {
    test.Run(context);
    completedOperations += 1;
    lastCompletionTimes = std::chrono::system_clock::now() - start;
  }
}

inline std::string FormatNumber(double const& number, bool showDecimals = true)
{
  auto fullString = std::to_string(number);
  auto dot = fullString.find('.');
  auto numberString = std::string(fullString.begin(), fullString.begin() + dot);
  if (numberString.size() > 3)
  {
    size_t start = numberString.length() - 3;
    while (start > 0)
    {
      numberString.insert(start, ",");
      if (start < 3)
      {
        start = 0;
      }
      else
      {
        start -= 3;
      }
    }
  }

  if (showDecimals)
  {
    return numberString + std::string(fullString.begin() + dot, fullString.end());
  }
  return numberString;
}

template <class T> inline T Sum(std::vector<T> const& array)
{
  T s = 0;
  for (T item : array)
  {
    s += item;
  }
  return s;
}

inline std::vector<double> ZipAvg(
    std::vector<uint64_t> const& operations,
    std::vector<std::chrono::nanoseconds> const& timeResults)
{
  auto size = operations.size();
  std::vector<double> s(size);
  for (size_t index = 0; index != operations.size(); index++)
  {
    s[index] = operations[index] / std::chrono::duration<double>(timeResults[index]).count();
  }
  return s;
}

inline void RunTests(
    Azure::Core::Context const& context,
    std::vector<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>> const& tests,
    Azure::PerformanceStress::GlobalTestOptions const& options,
    std::string const& title,
    bool warmup = false)
{
  (void)title;
  auto parallelTestsCount = options.Parallel;
  auto durationInSeconds = warmup ? options.Warmup : options.Duration;
  // auto jobStatistics = warmup ? false : options.JobStatistics;
  // auto latency = warmup ? false : options.Latency;

  std::vector<uint64_t> completedOperations(parallelTestsCount);
  std::vector<std::chrono::nanoseconds> lastCompletionTimes(parallelTestsCount);

  /********************* Progress Reporter ******************************/
  Azure::Core::Context progresToken;
  uint64_t lastCompleted = 0;
  auto progressThread = std::thread(
      [&title, &completedOperations, &lastCompletionTimes, &lastCompleted, &progresToken]() {
        std::cout << "=== " << title << " ===" << std::endl
                  << "Current\t\tTotal\t\tAverage" << std::endl;
        while (!progresToken.IsCancelled())
        {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(1000ms);
          auto total = Sum(completedOperations);
          auto current = total - lastCompleted;
          auto avg = Sum(ZipAvg(completedOperations, lastCompletionTimes));
          lastCompleted = total;
          std::cout << current << "\t\t" << total << "\t\t" << avg << std::endl;
        }
      });

  /********************* parallel test creation ******************************/
  std::vector<std::thread> tasks(tests.size());
  auto deadLineSeconds = std::chrono::seconds(durationInSeconds);
  for (size_t index = 0; index != tests.size(); index++)
  {
    tasks[index] = std::thread(
        [index, &tests, &completedOperations, &lastCompletionTimes, &deadLineSeconds, &context]() {
          bool isCancelled = false;
          // Azure::Context is not good performer for checking cancellation inside the test loop
          auto manualCancellation = std::thread([&deadLineSeconds, &isCancelled] {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(deadLineSeconds);
            isCancelled = true;
          });

          RunLoop(
              context,
              *tests[index],
              completedOperations[index],
              lastCompletionTimes[index],
              false,
              isCancelled);

          manualCancellation.join();
        });
  }
  // Wait for all tests to complete setUp
  for (auto& t : tasks)
  {
    t.join();
  }

  // Stop progress
  progresToken.Cancel();
  progressThread.join();

  std::cout << std::endl << "=== Results ===";

  auto totalOperations = Sum(completedOperations);
  auto operationsPerSecond = Sum(ZipAvg(completedOperations, lastCompletionTimes));
  auto secondsPerOperation = 1 / operationsPerSecond;
  auto weightedAverageSeconds = totalOperations / operationsPerSecond;

  std::cout << std::endl
            << "Completed " << FormatNumber(totalOperations, false)
            << " operations in a weighted-average of "
            << FormatNumber(weightedAverageSeconds, false) << "s ("
            << FormatNumber(operationsPerSecond) << " ops/s, " << secondsPerOperation << " s/op)"
            << std::endl
            << std::endl;
}

} // namespace

void Azure::PerformanceStress::Program::Run(
    Azure::Core::Context const& context,
    std::map<
        std::string,
        std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::TestOptions)>> const& tests,
    int argc,
    char** argv)
{
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

  if (options.JobStatistics)
  {
    std::cout << "Application started." << std::endl;
  }

  // Print options
  PrintOptions(options, testOptions, argResults);

  // Create parallel pool of tests
  int const parallelTasks = options.Parallel;
  std::vector<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>> parallelTest(
      parallelTasks);
  for (int i = 0; i < parallelTasks; i++)
  {
    parallelTest[i] = testGenerator(Azure::PerformanceStress::TestOptions(argResults));
  }

  /******************** Global Set up ******************************/
  test->GlobalSetup();

  /******************** Set up ******************************/
  {
    std::vector<std::thread> tasks(parallelTasks);
    for (int i = 0; i < parallelTasks; i++)
    {
      tasks[i] = std::thread([&parallelTest, i]() { parallelTest[i]->Setup(); });
    }
    // Wait for all tests to complete setUp
    for (auto& t : tasks)
    {
      t.join();
    }
  }

  /******************** WarmUp ******************************/
  if (options.Warmup)
  {
    RunTests(context, parallelTest, options, "Warmup", true);
  }

  /******************** Tests ******************************/
  std::string iterationInfo;
  try
  {
    for (int iteration = 0; iteration < options.Iterations; iteration++)
    {
      if (iteration > 0)
      {
        iterationInfo.append(FormatNumber(iteration));
      }
      RunTests(context, parallelTest, options, "Test" + iterationInfo);
    }
  }
  catch (std::exception const& error)
  {
    std::cout << "Error: " << error.what();
  }

  /******************** Clean up ******************************/
  if (!options.NoCleanup)
  {
    std::cout << std::endl << "=== Cleanup ===" << std::endl;
    std::vector<std::thread> tasks(parallelTasks);
    for (int i = 0; i < parallelTasks; i++)
    {
      tasks[i] = std::thread([&parallelTest, i]() { parallelTest[i]->Cleanup(); });
    }
    for (auto& t : tasks)
    {
      t.join();
    }
    test->GlobalCleanup();
  }
}
