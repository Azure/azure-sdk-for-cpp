// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/options.hpp"

#include <iostream>

void Azure::Perf::to_json(Azure::Core::Json::_internal::json& j, const GlobalTestOptions& p)
{
  j = Azure::Core::Json::_internal::json{
      {"Duration", p.Duration},
      {"Host", p.Host},
      {"Insecure", p.Insecure},
      {"Iterations", p.Iterations},
      {"JobStatistics", p.JobStatistics},
      {"Latency", p.Latency},
      {"NoCleanup", p.NoCleanup},
      {"Parallel", p.Parallel},
      {"Warmup", p.Warmup},
      {"StatusInterval", p.StatusInterval},
      {"ResultsFile", p.ResultsFile.empty() ? "N/A" : p.ResultsFile}};
  if (p.Port)
  {
    j["Port"] = p.Port.Value();
  }
  else
  {
    j["Port"] = nullptr;
  }
  if (p.Rate)
  {
    j["Rate"] = p.Rate.Value();
  }
  else
  {
    j["Rate"] = nullptr;
  }
  if (p.TestProxies.empty())
  {
    j["TestProxies"] = "N/A";
  }
  else
  {
    j["TestProxies"] = p.TestProxies;
  }
}

std::vector<Azure::Perf::TestOption> Azure::Perf::GlobalTestOptions::GetOptionMetadata()
{
  /*
    [Option('d', "duration", Default = 10, HelpText = "Duration of test in seconds")]
    [Option("host", HelpText = "Host to redirect HTTP requests")]
    [Option("insecure", HelpText = "Allow untrusted SSL certs")]
    [Option('i', "iterations", Default = 1, HelpText = "Number of iterations of main test loop")]
    [Option("job-statistics", HelpText = "Print job statistics (used by automation)")]
    [Option('l', "latency", HelpText = "Track and print per-operation latency statistics")]
    [Option("no-cleanup", HelpText = "Disables test cleanup")]
    [Option('p', "parallel", Default = 1, HelpText = "Number of operations to execute in parallel")]
    [Option("port", HelpText = "Port to redirect HTTP requests")]
    [Option('r', "rate", HelpText = "Target throughput (ops/sec)")]
    [Option('w', "warmup", Default = 5, HelpText = "Duration of warmup in seconds")]
    [Option('x', "proxy", Default = "", HelpText = "Proxy server")]
  */
  return {
      {"Duration",
       {"-d", "--duration"},
       "Duration of the test in seconds. Default to 10 seconds.",
       1},
      {"help", {"-h", "--help"}, "Display help information.", 0},
      {"Host", {"--host"}, "Host to redirect HTTP requests. No redirection by default.", 1},
      {"Insecure", {"--insecure"}, "Allow untrusted SSL certs. Default to false.", 0},
      {"Iterations",
       {"-i", "--iterations"},
       "Number of iterations of main test loop. Default to 1.",
       1},
      {"JobStatistics", {"--statistics"}, "Print job statistics. Default to false", 1},
      {"Latency",
       {"-l", "--latency"},
       "Track and print per-operation latency statistics. Default to false.",
       1},
      {"NoCleanup", {"--noclean"}, "Disables test clean up. Default to false.", 1},
      // .NET-compatible bare-switch alias for --noclean.
      {"NoCleanupSwitch",
       {"--no-cleanup"},
       "Disables test clean up (bare switch, matches .NET --no-cleanup).",
       0},
      {"Parallel",
       {"-p", "--parallel"},
       "Number of operations to execute in parallel. Default to 1.",
       1},
      {"Port", {"--port"}, "Port to redirect HTTP requests. Default to no redirection.", 1},
      {"Rate", {"-r", "--rate"}, "Target throughput (ops/sec). Default to no throughput.", 1},

      // Accepted for cross-language CLI compatibility (perf-automation appends --sync for
      // sync-only languages). C++ is sync-only and has no async variant, so the flag is
      // parsed and intentionally ignored.
      {"Sync", {"-y", "--sync"}, "Accepted for compatibility; ignored (C++ is sync-only).", 0},
      {"TestProxies", {"-x", "--test-proxies"}, "URIs of TestProxy Servers (separated by ';')", 1},
      {"Warmup", {"-w", "--warmup"}, "Duration of warmup in seconds. Default to 5 seconds.", 1},
      {"StatusInterval",
       {"--status-interval"},
       "Interval in seconds between live status lines. Default to 1.",
       1},
      {"ResultsFile",
       {"--results-file"},
       "Write per-operation results ({Time, Size}) as JSON to this file. Requires --latency.",
       1},
  };
}
