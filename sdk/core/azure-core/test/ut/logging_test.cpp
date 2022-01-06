// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/diagnostics/log.hpp>
#include <gtest/gtest.h>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

TEST(Logger, Levels)
{
  Logger::SetListener([](auto, auto) {});

  Logger::SetLevel(Logger::Level::Verbose);
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Verbose));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Informational));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Warning));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Informational);
  EXPECT_FALSE(Log::ShouldWrite(Logger::Level::Verbose));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Informational));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Warning));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Warning);
  EXPECT_FALSE(Log::ShouldWrite(Logger::Level::Verbose));
  EXPECT_FALSE(Log::ShouldWrite(Logger::Level::Informational));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Warning));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Error);
  EXPECT_FALSE(Log::ShouldWrite(Logger::Level::Verbose));
  EXPECT_FALSE(Log::ShouldWrite(Logger::Level::Informational));
  EXPECT_FALSE(Log::ShouldWrite(Logger::Level::Warning));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Verbose);
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Verbose));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Informational));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Warning));
  EXPECT_TRUE(Log::ShouldWrite(Logger::Level::Error));

  Logger::SetListener(nullptr);
}

TEST(Logger, Message)
{
  try
  {
    Logger::Level level = Logger::Level::Error;
    std::string message;

    Logger::SetListener([&](auto lvl, auto msg) {
      level = lvl;
      message = msg;
    });

    Logger::SetLevel(Logger::Level::Verbose);
    {
      level = Logger::Level::Error;
      message = "";

      Log::Write(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Verbose);
      EXPECT_EQ(message, "Verbose");

      Log::Write(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Informational);
      EXPECT_EQ(message, "Informational");

      Log::Write(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log::Write(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(Logger::Level::Informational);
    {
      level = Logger::Level::Error;
      message = "";

      Log::Write(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Informational);
      EXPECT_EQ(message, "Informational");

      Log::Write(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log::Write(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(Logger::Level::Warning);
    {
      level = Logger::Level::Error;
      message = "";

      Log::Write(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log::Write(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(Logger::Level::Error);
    {
      level = Logger::Level::Error;
      message = "";

      Log::Write(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      level = Logger::Level::Verbose;

      Log::Write(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    // Verify that we can switch back to Verbose
    Logger::SetLevel(Logger::Level::Verbose);
    {
      level = Logger::Level::Error;
      message = "";

      Log::Write(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Verbose);
      EXPECT_EQ(message, "Verbose");

      Log::Write(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Informational);
      EXPECT_EQ(message, "Informational");

      Log::Write(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log::Write(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetListener(nullptr);

    Logger::SetLevel(Logger::Level::Verbose);
    {
      level = Logger::Level::Error;
      message = "";

      Log::Write(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log::Write(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      level = Logger::Level::Verbose;

      Log::Write(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Verbose);
      EXPECT_EQ(message, "");
    }
  }
  catch (...)
  {
    Logger::SetListener(nullptr);
    throw;
  }
}

namespace {
struct LoggerTestParameter
{
  std::string Suffix;
  uint64_t LogLines;
};

class LoggerTest : public testing::TestWithParam<LoggerTestParameter> {

protected:
  virtual void SetUp() override {}
};

std::string GetSuffix(const testing::TestParamInfo<LoggerTest::ParamType>& info)
{
  return info.param.Suffix;
}

std::vector<LoggerTestParameter> GetTestParameters()
{
  std::vector<LoggerTestParameter> parameters;
  parameters.emplace_back(LoggerTestParameter{"halfMillion", 500000U});
  parameters.emplace_back(LoggerTestParameter{"oneMillion", 1000000U});
  return parameters;
}

// cspell:disable
constexpr static const char LogSample[]
    = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt "
      "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
      "ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
      "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur "
      "sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est "
      "laborum.";
// cspell:enable

} // namespace

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

TEST_P(LoggerTest, defaultLogger)
{
  LoggerTestParameter parameter(GetParam());

  for (uint64_t counter = 0; counter < parameter.LogLines; counter++)
  {
    Log::Write(Logger::Level::Informational, LogSample);
  }
}

#include <thread>
#include <vector>

TEST_P(LoggerTest, parallelLog)
{
  LoggerTestParameter parameter(GetParam());

  std::vector<std::thread> workers;

  auto routine = [parameter]() {
    for (uint64_t counter = 0; counter < parameter.LogLines; counter++)
    {
      Log::Write(Logger::Level::Informational, LogSample);
    }
  };

  for (auto j = 0; j < 5; j++)
  {
    workers.emplace_back(std::thread(routine));
  }
  for (auto& w : workers)
  {
    w.join();
  }
}

INSTANTIATE_TEST_SUITE_P(withParam, LoggerTest, testing::ValuesIn(GetTestParameters()), GetSuffix);
