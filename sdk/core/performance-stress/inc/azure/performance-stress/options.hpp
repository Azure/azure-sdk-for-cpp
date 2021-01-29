// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "azure/core/nullable.hpp"
#include "azure/performance-stress/argagg.hpp"
#include "azure/performance-stress/test_options.hpp"
#include "azure/performance-stress/dynamic_test_options.hpp"

#include <azure/core/internal/json.hpp>

#include <iostream>
#include <vector>

namespace Azure { namespace PerformanceStress {
  // options supported when running a test.
  // TODO: add defaults for command line
  struct GlobalTestOptions
  {
    int Duration = 10;

    std::string Host;

    bool Insecure;

    int Iterations = 1;

    bool JobStatistics;

    bool Latency;

    bool NoCleanup;

    int Parallel = 1;

    Azure::Core::Nullable<int> Port;

    Azure::Core::Nullable<int> Rate;

    // bool Sync;  - Only sync on C++

    int Warmup = 5;

    static std::vector<Azure::PerformanceStress::TestOption> GetOptionMetadata();

    // template <class T> static T GetOptionOrDefault(std::string const& option, T def)
    // {
    //   for (auto r : argagg::Parser.ArgsResult.all_as<std::string>())
    //   {
    //     std::cout << r << "  " << option;
    //   }

    //   // if (argagg::ArgsResult["ExtraOption"])
    //   // {
    //   //   return argagg::ArgsResult[option].as<T>();
    //   // }
    //   return def;
    // }
  };

  void to_json(Azure::Core::Internal::Json::json& j, const GlobalTestOptions& p);
}} // namespace Azure::PerformanceStress
