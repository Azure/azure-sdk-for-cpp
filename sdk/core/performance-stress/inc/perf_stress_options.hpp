// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/core/nullable.hpp"

namespace Azure { namespace PerfStress {
  // options supported when running a test.
  // TODO: add defaults for command line
  struct PerfStressOptions
  {
    /* [Option('d', "duration", Default = 10, HelpText = "Duration of test in seconds")] */
    int Duration;

    /* [Option("host", HelpText = "Host to redirect HTTP requests")] */
    std::string Host;

    /* [Option("insecure", HelpText = "Allow untrusted SSL certs")] */
    bool Insecure;

    /* [Option('i', "iterations", Default = 1, HelpText = "Number of iterations of main test loop")]
     */
    int Iterations;

    /* [Option("job-statistics", HelpText = "Print job statistics (used by automation)")] */
    bool JobStatistics;

    /* [Option('l', "latency", HelpText = "Track and print per-operation latency statistics")] */
    bool Latency;

    /* [Option("no-cleanup", HelpText = "Disables test cleanup")] */
    bool NoCleanup;

    /* [Option('p', "parallel", Default = 1, HelpText = "Number of operations to execute in
     * parallel")] */
    int Parallel;

    /* [Option("port", HelpText = "Port to redirect HTTP requests")] */
    Azure::Core::Nullable<int> Port;

    /* [Option('r', "rate", HelpText = "Target throughput (ops/sec)")] */
    Azure::Core::Nullable<int> Rate;

    /* [Option("sync", HelpText = "Runs sync version of test")] */
    bool Sync;

    /* [Option('w', "warmup", Default = 5, HelpText = "Duration of warmup in seconds")] */
    int Warmup;
  };
}} // namespace Azure::PerfStress
