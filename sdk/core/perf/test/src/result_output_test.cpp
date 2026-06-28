// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/perf/result_output.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

using Azure::Perf::OperationResult;
using Azure::Perf::RunSummary;

namespace {
std::string TempPath(std::string const& tag)
{
  auto base = ::testing::TempDir();
  if (!base.empty() && base.back() != '/' && base.back() != '\\')
  {
#ifdef _WIN32
    base += '\\';
#else
    base += '/';
#endif
  }
  return base + "azure_perf_test_" + tag + ".json";
}

bool FileExists(std::string const& path)
{
  std::ifstream f(path.c_str());
  return f.good();
}

std::string Slurp(std::string const& path)
{
  std::ifstream f(path.c_str(), std::ios::binary);
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}
} // namespace

TEST(result_output, write_results_file_matches_dotnet_schema)
{
  auto path = TempPath("results");
  std::vector<OperationResult> results;
  results.push_back({12.5, 1024});
  results.push_back({8.0, 1024});
  Azure::Perf::WriteResultsFile(path, results);
  EXPECT_TRUE(FileExists(path));
  auto contents = Slurp(path);
  // .NET OperationResult JSON shape: { "Time": ..., "Size": ... }.
  // Field names are PascalCase and must match the .NET reference framework exactly.
  EXPECT_NE(contents.find("\"Time\""), std::string::npos);
  EXPECT_NE(contents.find("\"Size\""), std::string::npos);
  EXPECT_NE(contents.find("12.5"), std::string::npos);
  EXPECT_NE(contents.find("1024"), std::string::npos);
  // Schema must NOT contain the legacy / Go-style field names.
  EXPECT_EQ(contents.find("\"operation\""), std::string::npos);
  EXPECT_EQ(contents.find("\"latencyMs\""), std::string::npos);
  EXPECT_EQ(contents.find("\"sizeBytes\""), std::string::npos);
  std::remove(path.c_str());
}

TEST(result_output, print_job_statistics_matches_dotnet_envelope)
{
  RunSummary s;
  s.OperationsPerSecond = 1234.5;

  // Capture std::cout.
  std::stringstream buffer;
  auto* oldBuf = std::cout.rdbuf(buffer.rdbuf());
  Azure::Perf::PrintJobStatistics(s);
  std::cout.rdbuf(oldBuf);

  auto out = buffer.str();
  EXPECT_NE(out.find("#StartJobStatistics"), std::string::npos);
  EXPECT_NE(out.find("#EndJobStatistics"), std::string::npos);
  // Match the .NET BenchmarkOutput envelope AND key order:
  // { "Metadata": [...], "Measurements": [...] } -- Metadata must appear before Measurements.
  std::size_t metaPos = out.find("\"Metadata\"");
  std::size_t measPos = out.find("\"Measurements\"");
  EXPECT_NE(metaPos, std::string::npos);
  EXPECT_NE(measPos, std::string::npos);
  EXPECT_LT(metaPos, measPos);
  EXPECT_NE(out.find("\"Source\""), std::string::npos);
  EXPECT_NE(out.find("PerfStress"), std::string::npos);
  EXPECT_NE(out.find("perfstress/throughput"), std::string::npos);
  EXPECT_NE(out.find("1234.5"), std::string::npos);
}
