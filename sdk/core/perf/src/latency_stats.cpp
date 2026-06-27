// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/latency_stats.hpp"

#include <algorithm>
#include <map>

namespace {

double NanosToMs(std::chrono::nanoseconds ns)
{
  return std::chrono::duration<double, std::milli>(ns).count();
}

// Compute the value at percentile `p` (0..100) using nearest-rank.
// `sortedMs` must be sorted ascending. Returns 0 for an empty input.
double Percentile(std::vector<double> const& sortedMs, double p)
{
  if (sortedMs.empty())
  {
    return 0.0;
  }
  if (sortedMs.size() == 1)
  {
    return sortedMs.front();
  }
  double rank = (p / 100.0) * static_cast<double>(sortedMs.size() - 1);
  size_t lo = static_cast<size_t>(rank);
  size_t hi = (lo + 1 < sortedMs.size()) ? lo + 1 : lo;
  double frac = rank - static_cast<double>(lo);
  return sortedMs[lo] * (1.0 - frac) + sortedMs[hi] * frac;
}

Azure::Perf::LatencyCollector::Summary SummaryFromMs(std::vector<double>& msValues)
{
  Azure::Perf::LatencyCollector::Summary s;
  s.Count = msValues.size();
  if (msValues.empty())
  {
    return s;
  }
  std::sort(msValues.begin(), msValues.end());
  double sum = 0;
  for (auto v : msValues)
  {
    sum += v;
  }
  s.MeanMs = sum / static_cast<double>(msValues.size());
  s.P50Ms = Percentile(msValues, 50);
  s.P75Ms = Percentile(msValues, 75);
  s.P90Ms = Percentile(msValues, 90);
  s.P99Ms = Percentile(msValues, 99);
  s.P999Ms = Percentile(msValues, 99.9);
  s.P9999Ms = Percentile(msValues, 99.99);
  s.P99999Ms = Percentile(msValues, 99.999);
  s.P100Ms = msValues.back();
  return s;
}

} // namespace

namespace Azure { namespace Perf {

  void LatencyCollector::Record(std::chrono::nanoseconds duration)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_samples.push_back(Sample{duration, std::string{}});
  }

  void LatencyCollector::Record(std::string const& callType, std::chrono::nanoseconds duration)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_samples.push_back(Sample{duration, callType});
  }

  void LatencyCollector::Reset()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_samples.clear();
  }

  LatencyCollector::Summary LatencyCollector::Summarize() const
  {
    std::vector<Sample> snapshot;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      snapshot = m_samples;
    }
    std::vector<double> msValues;
    msValues.reserve(snapshot.size());
    for (auto const& s : snapshot)
    {
      msValues.push_back(NanosToMs(s.Duration));
    }
    return SummaryFromMs(msValues);
  }

  std::vector<std::pair<std::string, LatencyCollector::Summary>>
  LatencyCollector::SummarizeByCallType() const
  {
    std::vector<Sample> snapshot;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      snapshot = m_samples;
    }
    std::map<std::string, std::vector<double>> buckets;
    for (auto const& s : snapshot)
    {
      buckets[s.CallType].push_back(NanosToMs(s.Duration));
    }
    std::vector<std::pair<std::string, Summary>> result;
    for (auto& kv : buckets)
    {
      result.emplace_back(kv.first, SummaryFromMs(kv.second));
    }
    return result;
  }

  std::vector<LatencyCollector::Sample> LatencyCollector::Samples() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_samples;
  }

}} // namespace Azure::Perf
