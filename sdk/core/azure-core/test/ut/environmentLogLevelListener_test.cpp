// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include <private/environment_log_level_listener.hpp>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;

namespace {

std::string environmentVariable = "AZURE_LOG_LEVEL";

void SetLogLevel(std::string const& value)
{
#if defined(_MSC_VER)
  static_cast<void>(_putenv((environmentVariable + "=" + value).c_str()));
#else
  static_cast<void>(setenv(environmentVariable.c_str(), value.c_str(), 1));
#endif
}

} // namespace
class EnvironmentLogLevelListenerTest : public testing::Test {
protected:
  void SetUp() override
  {
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
    auto const value = std::getenv(environmentVariable.c_str());
    m_previousValue = value == nullptr ? "" : value;
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  }

  void TearDown() override { SetLogLevel(m_previousValue); }

private:
  std::string m_previousValue;
};

TEST_F(EnvironmentLogLevelListenerTest, LogLevelDefault)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("unknown");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("error");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("err");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);

  SetLogLevel("4");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Error);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("warning");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("warn");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("3");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Warning);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("informational");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("info");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("information");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("2");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Informational);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("debug");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("1");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, Logger::Level::Verbose);
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Verbose, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("DEBUG : message"), std::string::npos);
  std::cerr.rdbuf(old);
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Error, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("ERROR : message"), std::string::npos);
  std::cerr.rdbuf(old);
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Warning, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("WARN  : message"), std::string::npos);
  std::cerr.rdbuf(old);
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  listener(Logger::Level::Informational, "message");
  EXPECT_NE(listener, nullptr);
  EXPECT_NE(buffer.str().find("INFO  : message"), std::string::npos);
  std::cerr.rdbuf(old);
}
