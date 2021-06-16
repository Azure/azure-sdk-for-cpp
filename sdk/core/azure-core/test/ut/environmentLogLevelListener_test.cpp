// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include <private/environment_log_level_listener.hpp>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;

namespace {
void SetVariable(std::string const& value)
{
#if defined(_MSC_VER)
  static_cast<void>(_putenv(("AZURE_LOG_LEVEL=" + value).c_str()));
#else
  static_cast<void>(setenv(std::string("AZURE_LOG_LEVEL").c_str(), value.c_str(), 1));
#endif
}
}// namespace


TEST(EnvironmentLogLevelListener, LogLevelDefault)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("unknown");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST(EnvironmentLogLevelListener, LogLevelError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("error");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("err");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  SetVariable("4");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);
}

TEST(EnvironmentLogLevelListener, LogLevelWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("warning");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("warn");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("3");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);
}

TEST(EnvironmentLogLevelListener, LogLevelInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("informational");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("info");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("information");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("2");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);
}

TEST(EnvironmentLogLevelListener, LogLevelVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("verbose");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("debug");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("1");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST(EnvironmentLogLevelListener, GetLogListenerVerbose) {
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Verbose, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("DEBUG : message"), std::string::npos);
  std::cerr.rdbuf(old);
}

TEST(EnvironmentLogLevelListener, GetLogListenerError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Error, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("ERROR : message"), std::string::npos);
  std::cerr.rdbuf(old);
}

TEST(EnvironmentLogLevelListener, GetLogListenerWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Warning, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("WARN  : message"), std::string::npos);
  std::cerr.rdbuf(old);
}

TEST(EnvironmentLogLevelListener, GetLogListenerInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetVariable("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Informational, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("INFO  : message"), std::string::npos);
  std::cerr.rdbuf(old);
}