// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <http/pipeline.hpp>
#include <internal/log.hpp>
#include <logging/logging.hpp>

#include <utility>
#include <vector>

using namespace Azure::Core;

namespace {
typedef std::vector<std::pair<Logging::LogClassification, std::string>> LogArguments;

struct LogRecorder
{
  LogArguments Actual;

  Logging::LogListener LogListener
      = [&](Logging::LogClassification const& c, std::string const& m) {
          Actual.push_back(std::make_pair(c, m));
        };
};
} // namespace

TEST(Logging, allClassifications)
{
  LogArguments const Values{
      {Http::LogClassification::Request, "Request"},
      {Http::LogClassification::Request, "Request"},
      {Http::LogClassification::Response, "Response"},
      {Http::LogClassification::Request, "Request"},
  };

  LogRecorder logRecorder;
  Logging::SetLogListener(logRecorder.LogListener);

  Logging::SetLogClassifications(LogClassification::All);
  auto const expected = Values;

  for (auto value : Values)
  {
    Logging::Details::Write(value.first, value.second);
  }

  EXPECT_EQ(logRecorder.Actual, expected);
}

TEST(Logging, filteredClassifications){
    LogArguments const Values{{Http::LogClassification::Request, "Request"},
                              {Http::LogClassification::Request, "Request"},
                              {Http::LogClassification::Response, "Response"},
                              {Http::LogClassification::Request, "Request"},
                              Http::LogClassification::Retry,
                              "Retry"},
};

LogRecorder logRecorder;
Logging::SetLogListener(logRecorder.LogListener);

auto const AllowedClassification1 = Http::LogClassification::Request;
auto const AllowedClassification2 = Http::LogClassification::Retry;
Logging::SetLogClassifications({AllowedClassification1, AllowedClassification2});

LogArguments expected;
for (auto value : Values)
{
  if (value.first == AllowedClassification1 || value.first == AllowedClassification2)
  {
    expected.add(value);
  }
}

for (auto value : Values)
{
  EXPECT_EQ(
      Logging::Details::ShouldWrite(value.first),
      (value.first == AllowedClassification1 || value.first == AllowedClassification2));

  Logging::Details::Write(value.first, value.second);
}

EXPECT_EQ(logRecorder.Actual, expected);
}

TEST(Logging, noClassifications)
{
  LogArguments const Values{
      {Http::LogClassification::Request, "Request"},
      {Http::LogClassification::Request, "Request"},
      {Http::LogClassification::Response, "Response"},
      {Http::LogClassification::Request, "Request"},
  };

  LogRecorder logRecorder;
  Logging::SetLogListener(logRecorder.LogListener);

  auto const AllowedClassification = Http::LogClassification::Request;
  Logging::SetLogClassifications(LogClassification::None);
  LogArguments const expected; // Empty

  for (auto value : Values)
  {
    EXPECT_EQ(Logging::Details::ShouldWrite(value.first), false);
    Logging::Details::Write(value.first, value.second);
  }

  EXPECT_EQ(logRecorder.Actual, expected);
}
