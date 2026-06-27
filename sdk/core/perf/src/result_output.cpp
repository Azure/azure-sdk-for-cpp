// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/result_output.hpp"

#include <azure/core/internal/json/json.hpp>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {

using Azure::Core::Json::_internal::json;
using Azure::Perf::OperationResult;
using Azure::Perf::RunSummary;

bool TryOpen(std::ofstream& f, std::string const& path)
{
  f.open(path, std::ios::out | std::ios::trunc);
  if (!f.is_open())
  {
    std::cerr << "warning: failed to open output file " << path << std::endl;
    return false;
  }
  return true;
}

// ISO-8601 UTC timestamp matching .NET's DateTime.ToString("O") JSON serialization,
// which emits fractional seconds at 100-nanosecond (7-digit) resolution.
std::string IsoUtcNow()
{
  using namespace std::chrono;
  auto now = system_clock::now();
  auto secs = time_point_cast<seconds>(now);
  // 100-nanosecond ticks within the current second, matching .NET DateTime "fffffff".
  // system_clock typically has microsecond resolution on Windows; pad with trailing zeros.
  auto ticks = duration_cast<duration<int64_t, std::ratio<1, 10000000>>>(now - secs).count();
  std::time_t tt = system_clock::to_time_t(secs);
  std::tm tm {};
#if defined(_WIN32)
  gmtime_s(&tm, &tt);
#else
  gmtime_r(&tt, &tm);
#endif
  std::ostringstream os;
  os << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "." << std::setw(7) << std::setfill('0')
     << ticks << "Z";
  return os.str();
}

} // namespace

namespace Azure { namespace Perf {

  void WriteResultsFile(std::string const& path, std::vector<OperationResult> const& results)
  {
    if (path.empty())
    {
      return;
    }
    std::ofstream f;
    if (!TryOpen(f, path))
    {
      return;
    }
    // Match the .NET Azure.Test.Perf OperationResult JSON shape exactly:
    // [ { "Time": <double ms>, "Size": <long bytes> }, ... ]
    json arr = json::array();
    for (auto const& r : results)
    {
      arr.push_back(json{{"Time", r.Time}, {"Size", r.Size}});
    }
    f << arr.dump(2) << std::endl;
  }

  void PrintJobStatistics(RunSummary const& summary)
  {
    // Match the .NET BenchmarkOutput shape AND key order exactly so perf-automation's
    // downstream parser sees the same fields as for .NET runs:
    //   { "Metadata": [ { Source, Name, ShortDescription, LongDescription, Format } ],
    //     "Measurements": [ { Timestamp, Name, Value } ] }
    // We serialize manually because nlohmann::json sorts object keys alphabetically by
    // default; .NET emits keys in declaration order.
    std::ostringstream os;
    os << "{\"Metadata\":[{"
       << "\"Source\":\"PerfStress\","
       << "\"Name\":\"perfstress/throughput\","
       << "\"ShortDescription\":\"Throughput (ops/sec)\","
       << "\"LongDescription\":\"Throughput (ops/sec)\","
       << "\"Format\":\"n2\""
       << "}],\"Measurements\":[{"
       << "\"Timestamp\":\"" << IsoUtcNow() << "\","
       << "\"Name\":\"perfstress/throughput\","
       << "\"Value\":" << json(summary.OperationsPerSecond).dump()
       << "}]}";
    std::cout << "#StartJobStatistics" << std::endl;
    std::cout << os.str() << std::endl;
    std::cout << "#EndJobStatistics" << std::endl;
  }

}} // namespace Azure::Perf
