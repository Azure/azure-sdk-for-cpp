// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef __APPLE__
#define PUTENV(x) putenv((char*)x)
#else
#define PUTENV(x) _putenv(std::string(x).c_str());
#endif // __APPLE__

#include <gtest/gtest.h>
#include <private/environment_log_level_listener.hpp>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;

TEST(EnvironmentLogLevelListener, LogLevelDefault)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=goqu");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST(EnvironmentLogLevelListener, LogLevelError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=error");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=err");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=4");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);
}

TEST(EnvironmentLogLevelListener, LogLevelWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=warning");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=warn");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=3");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);
}

TEST(EnvironmentLogLevelListener, LogLevelInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=informational");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=info");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=information");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=2");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);
}

TEST(EnvironmentLogLevelListener, LogLevelVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=verbose");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=debug");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV("AZURE_LOG_LEVEL=1");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}
