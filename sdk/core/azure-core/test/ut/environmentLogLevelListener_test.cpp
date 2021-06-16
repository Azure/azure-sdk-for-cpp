// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef __APPLE__
#define PUTENV putenv
#else
#define PUTENV _putenv
#endif // __APPLE__

#include <gtest/gtest.h>
#include <private/environment_log_level_listener.hpp>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;

TEST(EnvironmentLogLevelListener, LogLevelDefault)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=goqu").c_str());
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST(EnvironmentLogLevelListener, LogLevelError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=error").c_str());
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=err").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=4").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);
}

TEST(EnvironmentLogLevelListener, LogLevelWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=warning").c_str());
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=warn").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=3").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);
}

TEST(EnvironmentLogLevelListener, LogLevelInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=informational").c_str());
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=info").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=information").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=2").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);
}

TEST(EnvironmentLogLevelListener, LogLevelVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=verbose").c_str());
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=debug").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  PUTENV(std::string("AZURE_LOG_LEVEL=1").c_str());
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}