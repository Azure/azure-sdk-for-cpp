// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/performance-stress/options.hpp"

#include <iostream>

void Azure::PerformanceStress::to_json(Azure::Core::Internal::Json::json& j, const Options& p)
{
  j = Azure::Core::Internal::Json::json{
      {"Duration", p.Duration},
      {"Host", p.Host},
      {"Insecure", p.Insecure},
      {"Iterations", p.Iterations},
      {"JobStatistics", p.JobStatistics},
      {"Latency", p.Latency},
      {"NoCleanup", p.NoCleanup},
      {"Parallel", p.Parallel},
      {"Sync", p.Sync},
      {"Warmup", p.Warmup}};
  if (p.Port)
  {
    j["Port"] = p.Port.GetValue();
  }
  else
  {
    j["Port"] = "Null";
  }
  if (p.Rate)
  {
    j["Rate"] = p.Rate.GetValue();
  }
  else
  {
    j["Rate"] = "Null";
  }
}
