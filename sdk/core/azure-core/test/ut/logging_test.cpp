// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/diagnostics/log.hpp>
#include <gtest/gtest.h>
#include <private/environment_log_level_listener.hpp>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;
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

TEST(EnvironmentLogLevelListener, LogLevelDefault)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=goqu");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST(EnvironmentLogLevelListener, LogLevelError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=error");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=err");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=4");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);
}

TEST(EnvironmentLogLevelListener, LogLevelWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=warning");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=warn");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=3");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);
}

TEST(EnvironmentLogLevelListener, LogLevelInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=informational");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=info");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=information");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=2");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);
}

TEST(EnvironmentLogLevelListener, LogLevelVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=verbose");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=debug");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  _putenv("AZURE_LOG_LEVEL=1");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
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
