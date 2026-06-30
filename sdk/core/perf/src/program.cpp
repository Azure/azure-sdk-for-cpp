// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/program.hpp"

#include "azure/perf/argagg.hpp"
#include "azure/perf/latency_stats.hpp"
#include "azure/perf/result_output.hpp"

#include <azure/core/internal/diagnostics/global_exception.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>
#include <azure/core/platform.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace {

inline void PrintAvailableTests(std::vector<Azure::Perf::TestMetadata> const& tests)
{
  std::cout << "No test name found in the input. Available tests to run:" << std::endl;
  std::cout << std::endl << "Name\t\tDescription" << std::endl << "---\t\t---" << std::endl;
  for (auto const& test : tests)
  {
    std::cout << test.Name << "\t\t" << test.Description << std::endl;
  }
}

inline Azure::Perf::TestMetadata const* GetTestMetadata(
    std::vector<Azure::Perf::TestMetadata> const& tests,
    int argc,
    char** argv)
{
  if (argc == 1)
  {
    return nullptr;
  }
  argagg::parser argParser;
  auto args = argParser.parse(argc, argv, true);

  if (!args.pos.empty())
  {
    auto testName = std::string(args.pos[0]);

    for (auto& test : tests)
    {
      if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
              test.Name, testName))
      {
        return &test;
      }
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
    Azure::Perf::GlobalTestOptions const& options,
    std::vector<Azure::Perf::TestOption> const& testOptions,
    argagg::parser_results const& parsedArgs)
{
  {
    std::cout << std::endl << "=== Global Options ===" << std::endl;
    Azure::Core::Json::_internal::json optionsAsJson = options;
    std::cout << ReplaceAll(optionsAsJson.dump(), ",", ",\n") << std::endl;
  }

  if (testOptions.size() > 0)
  {
    std::cout << std::endl << "=== Test Options ===" << std::endl;
    Azure::Core::Json::_internal::json optionsAsJson;
    for (auto const& option : testOptions)
    {
      try
      {
        if (parsedArgs.has_option(option.Name))
        {
          auto optionName{option.Name};
          if (option.ExpectedArgs != 0)
          {
            optionsAsJson[optionName]
                = (option.SensitiveData ? "***" : parsedArgs[optionName].as<std::string>());
          }
          else
          {
            optionsAsJson[optionName] = true;
          }
        }
        else
        {
          if (option.Required)
          {
            // re-throw
            throw std::invalid_argument("Missing mandatory parameter: " + option.Name);
          }
          else
          {
            if (option.ExpectedArgs == 0)
            {
              // arg was not parsed
              optionsAsJson[option.Name] = false;
            }
            else
            {
              // arg was not parsed
              optionsAsJson[option.Name] = nullptr;
            }
          }
        }
      }
      catch (std::out_of_range const&)
      {
        if (!option.Required)
        {
          // arg was not parsed
          optionsAsJson[option.Name] = nullptr;
        }
        else
        {
          // re-throw
          throw std::invalid_argument("Missing mandatory parameter: " + option.Name);
        }
      }
      catch (std::exception const&)
      {
        throw;
      }
    }
    std::cout << ReplaceAll(optionsAsJson.dump(), ",", ",\n") << std::endl << std::endl;
  }
}

inline void RunLoop(
    Azure::Core::Context const& context,
    Azure::Perf::PerfTest& test,
    uint64_t& completedOperations,
    std::chrono::nanoseconds& lastCompletionTimes,
    bool latency,
    Azure::Perf::LatencyCollector* latencyCollector,
    bool& isCancelled)
{
  auto start = std::chrono::system_clock::now();
  while (!isCancelled)
  {
    if (latency && latencyCollector != nullptr)
    {
      auto opStart = std::chrono::steady_clock::now();
      test.Run(context);
      auto opEnd = std::chrono::steady_clock::now();
      latencyCollector->Record(opEnd - opStart);
    }
    else
    {
      test.Run(context);
    }
    completedOperations += 1;
    lastCompletionTimes = std::chrono::system_clock::now() - start;
  }
}

template <class T> inline std::string FormatNumber(T const& number, bool showDecimals = true)
{
  auto fullString = std::to_string(number);
  auto dot = fullString.find('.');
  auto numberString = fullString;
  if (dot != std::string::npos)
  {
    numberString = std::string(fullString.begin(), fullString.begin() + dot);
  }
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

  if (showDecimals && dot != std::string::npos)
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
    std::vector<std::unique_ptr<Azure::Perf::PerfTest>> const& tests,
    Azure::Perf::GlobalTestOptions const& options,
    std::string const& title,
    Azure::Perf::LatencyCollector* latencyCollector,
    Azure::Perf::RunSummary* outSummary,
    bool warmup = false)
{
  auto parallelTestsCount = options.Parallel;
  auto durationInSeconds = warmup ? options.Warmup : options.Duration;
  auto recordLatency = warmup ? false : options.Latency;

  std::vector<uint64_t> completedOperations(parallelTestsCount);
  std::vector<std::chrono::nanoseconds> lastCompletionTimes(parallelTestsCount);

  // Per-iteration reset: clear the latency collector so each iteration produces an
  // independent summary, matching the Go perf-framework lifecycle.
  if (recordLatency && latencyCollector != nullptr)
  {
    latencyCollector->Reset();
  }

  /********************* Progress Reporter ******************************/
  Azure::Core::Context progressToken;
  uint64_t lastCompleted = 0;
  int statusInterval = (options.StatusInterval > 0) ? options.StatusInterval : 1;
  auto progressThread = std::thread([&title,
                                     &completedOperations,
                                     &lastCompletionTimes,
                                     &lastCompleted,
                                     &progressToken,
                                     statusInterval]() {
    std::cout << std::endl
              << "=== " << title << " ===" << std::endl
              << "Current\t\tTotal\t\tAverage" << std::endl;
    while (!progressToken.IsCancelled())
    {
      std::this_thread::sleep_for(std::chrono::seconds(statusInterval));
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
    tasks[index] = std::thread([index,
                                &tests,
                                &completedOperations,
                                &lastCompletionTimes,
                                &deadLineSeconds,
                                &context,
                                latencyCollector,
                                recordLatency]() {
      bool isCancelled = false;
      // Azure::Context is not good performer for checking cancellation inside the test loop
      auto manualCancellation = std::thread([&deadLineSeconds, &isCancelled] {
        std::this_thread::sleep_for(deadLineSeconds);
        isCancelled = true;
      });

      RunLoop(
          context,
          *tests[index],
          completedOperations[index],
          lastCompletionTimes[index],
          recordLatency,
          latencyCollector,
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
  progressToken.Cancel();
  progressThread.join();

  std::cout << std::endl << "=== Results ===";

  auto totalOperations = Sum(completedOperations);
  auto operationsPerSecond = Sum(ZipAvg(completedOperations, lastCompletionTimes));
  auto secondsPerOperation = 1 / operationsPerSecond;
  auto weightedAverageSeconds = totalOperations / operationsPerSecond;

  // Match the established `Completed N operations in a weighted-average of Ts (X ops/s,
  // Y s/op)` line format that downstream tools (Cpp.cs's ops/s regex) key off.
  std::cout << std::endl
            << "Completed " << FormatNumber(totalOperations, false)
            << " operations in a weighted-average of "
            << FormatNumber(weightedAverageSeconds, false) << "s ("
            << FormatNumber(operationsPerSecond) << " ops/s, " << secondsPerOperation << " s/op)"
            << std::endl
            << std::endl;

  if (!warmup && outSummary != nullptr)
  {
    outSummary->TotalOperations = totalOperations;
    outSummary->OperationsPerSecond = operationsPerSecond;
    outSummary->SecondsPerOperation = secondsPerOperation;
    outSummary->WeightedAverageSeconds = weightedAverageSeconds;
    if (recordLatency && latencyCollector != nullptr)
    {
      outSummary->Latency = latencyCollector->Summarize();
      outSummary->LatencyByCallType = latencyCollector->SummarizeByCallType();

      auto const& s = outSummary->Latency;
      if (s.Count > 0)
      {
        // Match the .NET Azure.Test.Perf latency distribution exactly:
        // format string is `{percentile,7:N3}%   {ms,8:N2}ms` -- i.e., 7-char-wide
        // percentile with 3 decimals, then "%   ", then 8-char-wide latency with
        // 2 decimals, then "ms". Reproduce the format here for byte-near parity.
        struct Row
        {
          double Pct;
          double Ms;
        };
        Row rows[] = {
            {50.0, s.P50Ms},
            {75.0, s.P75Ms},
            {90.0, s.P90Ms},
            {99.0, s.P99Ms},
            {99.9, s.P999Ms},
            {99.99, s.P9999Ms},
            {99.999, s.P99999Ms},
            {100.0, s.P100Ms},
        };
        std::cout << "=== Latency Distribution ===" << std::endl;
        for (auto const& row : rows)
        {
          std::ostringstream pctSs;
          pctSs << std::fixed << std::setprecision(3) << row.Pct;
          std::ostringstream msSs;
          msSs << std::fixed << std::setprecision(2) << row.Ms;
          std::cout << std::right << std::setw(7) << pctSs.str() << "%   " << std::right
                    << std::setw(8) << msSs.str() << "ms" << std::endl;
        }
      }
    }
  }
}

} // namespace

void Azure::Perf::Program::Run(
    Azure::Core::Context const& context,
    std::vector<Azure::Perf::TestMetadata> const& tests,
    int argc,
    char** argv)
{
  // Ensure that all calls to abort() no longer pop up a modal dialog on Windows.
#if defined(_DEBUG) && defined(_MSC_VER)
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

  // Declare a signal handler to report unhandled exceptions on Windows - this is not needed for
  // other OS's as they will print the exception to stderr in their terminate() function.
#if defined(AZ_PLATFORM_WINDOWS)
  signal(SIGABRT, Azure::Core::Diagnostics::_internal::GlobalExceptionHandler::HandleSigAbort);
#endif // AZ_PLATFORM_WINDOWS

  // Parse args only to get the test name first
  auto testMetadata = GetTestMetadata(tests, argc, argv);
  if (testMetadata == nullptr)
  {
    // Wrong input. Print what are the options.
    PrintAvailableTests(tests);

    return;
  }
  auto const& testGenerator = testMetadata->Factory;

  // Initial test to get it's options, we can use a dummy parser results
  argagg::parser_results argResults;
  auto test = testGenerator(Azure::Perf::TestOptions(argResults));
  auto testOptions = test->GetTestOptions();
  argResults = Azure::Perf::Program::ArgParser::Parse(argc, argv, testOptions);
  // ReCreate Test with parsed results
  test = testGenerator(Azure::Perf::TestOptions(argResults));
  GlobalTestOptions options = Azure::Perf::Program::ArgParser::Parse(argResults);

  if (options.JobStatistics)
  {
    std::cout << std::endl << "Application started." << std::endl;
  }

  // Print test metadata
  std::cout << std::endl << "Running test: " << testMetadata->Name;
  std::cout << std::endl << "Description: " << testMetadata->Description << std::endl;

  // Print options
  PrintOptions(options, testOptions, argResults);

  // Create parallel pool of tests
  int const parallelTasks = options.Parallel;
  std::vector<std::unique_ptr<Azure::Perf::PerfTest>> parallelTest(parallelTasks);
  for (int i = 0; i < parallelTasks; i++)
  {
    parallelTest[i] = testGenerator(Azure::Perf::TestOptions(argResults));
    // Let the test know it should use a proxy
    if (!options.TestProxies.empty())
    {
      // Take the corresponding proxy from the list in round robin
      parallelTest[i]->SetTestProxy(options.TestProxies[i % options.TestProxies.size()]);
    }
    if (options.Insecure)
    {
      parallelTest[i]->AllowInsecureConnections(true);
    }
  }

  /******************** Per-run latency collector (when --latency) ****************/
  Azure::Perf::LatencyCollector latencyCollector;

  /******************** Global Set up ******************************/
  std::cout << std::endl << "=== Global Setup ===" << std::endl;
  test->GlobalSetup();

  std::cout << std::endl << "=== Test Setup ===" << std::endl;

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

  // instrument test for recordings if the env is set up.
  std::cout << std::endl << "=== Post Setup ===" << std::endl;
  {
    if (!options.TestProxies.empty())
    {
      std::cout << " - Creating test recordings for each test using test-proxies..." << std::endl;
      std::cout << " - Enabling test-proxy playback" << std::endl;
    }

    std::vector<std::thread> tasks(parallelTasks);
    for (int i = 0; i < parallelTasks; i++)
    {
      tasks[i] = std::thread([&parallelTest, i]() { parallelTest[i]->PostSetUp(); });
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
    RunTests(context, parallelTest, options, "Warmup", nullptr, nullptr, true);
  }

  /******************** Tests ******************************/
  std::string iterationInfo;
  Azure::Perf::RunSummary finalSummary;
  finalSummary.TestName = testMetadata->Name;
  finalSummary.Parallel = options.Parallel;
  finalSummary.DurationSeconds = options.Duration;
  finalSummary.Warmup = options.Warmup;
  finalSummary.Iterations = options.Iterations;
  for (int iteration = 0; iteration < options.Iterations; iteration++)
  {
    if (iteration > 0)
    {
      iterationInfo.append(FormatNumber(iteration));
    }
    RunTests(
        context,
        parallelTest,
        options,
        "Test" + iterationInfo,
        options.Latency ? &latencyCollector : nullptr,
        &finalSummary);
  }

  /******************** End-of-run artifacts ************************/
  if (options.Latency && !options.ResultsFile.empty())
  {
    // Match the .NET `--results-file` shape: an array of OperationResult { Time, Size }.
    // Time is per-op latency in ms; Size is taken from the test's --size option when
    // present, otherwise -1 (mirroring .NET's `(options as SizeOptions)?.Size ?? -1`).
    int64_t opSize = -1;
    try
    {
      if (argResults["Size"])
      {
        opSize = argResults["Size"].as<int64_t>();
      }
    }
    catch (std::exception const&)
    {
      opSize = -1;
    }

    std::vector<Azure::Perf::OperationResult> ops;
    auto samples = latencyCollector.Samples();
    ops.reserve(samples.size());
    for (auto const& s : samples)
    {
      Azure::Perf::OperationResult r;
      r.Time = std::chrono::duration<double, std::milli>(s.Duration).count();
      r.Size = opSize;
      ops.push_back(std::move(r));
    }
    Azure::Perf::WriteResultsFile(options.ResultsFile, ops);
  }

  if (options.JobStatistics)
  {
    Azure::Perf::PrintJobStatistics(finalSummary);
  }

  std::cout << std::endl << "=== Pre-Cleanup ===" << std::endl;
  {
    if (!options.TestProxies.empty())
    {
      std::cout << " - Deleting test recordings from test-proxies..." << std::endl;
      std::cout << " - Disabling test-proxy playback" << std::endl;
    }

    std::vector<std::thread> tasks(parallelTasks);
    for (int i = 0; i < parallelTasks; i++)
    {
      tasks[i] = std::thread([&parallelTest, i]() { parallelTest[i]->PreCleanUp(); });
    }
    // Wait for all tests to complete setUp
    for (auto& t : tasks)
    {
      t.join();
    }
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
