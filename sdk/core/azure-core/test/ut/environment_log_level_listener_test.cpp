// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

#include <gtest/gtest.h>
#include <private/environment_log_level_listener.hpp>

using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;

namespace {

constexpr bool IsUwp =
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
    false
#else
    true
#endif
    ;

std::string environmentVariable = "AZURE_LOG_LEVEL";

void SetLogLevel(std::string const& value)
{
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
#if defined(_MSC_VER)
  static_cast<void>(_putenv((environmentVariable + "=" + value).c_str()));
#else
  static_cast<void>(setenv(environmentVariable.c_str(), value.c_str(), 1));
#endif
#else
  static_cast<void>(value); // On UWP, there is no environment.
#endif
}

} // namespace
class EnvironmentLogLevelListenerTest : public testing::Test {
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
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

#endif
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
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Error);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("err");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Error);

  SetLogLevel("4");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Error);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("warning");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("warn");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Warning);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("3");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Warning);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("informational");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("info");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("information");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Informational);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("2");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Verbose);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Verbose : Logger::Level::Informational);
}

TEST_F(EnvironmentLogLevelListenerTest, LogLevelVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");
  auto level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Error);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Error : Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("debug");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Error);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Error : Logger::Level::Verbose);

  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("1");
  level = EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Error);
  EXPECT_EQ(level, IsUwp ? Logger::Level::Error : Logger::Level::Verbose);
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerVerbose)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  if (!IsUwp)
  {
    EXPECT_NE(listener, nullptr);

    listener(Logger::Level::Verbose, "message");
    EXPECT_NE(buffer.str().find("DEBUG : message"), std::string::npos);
    std::cerr.rdbuf(old);
  }
  else
  {
    EXPECT_EQ(listener, nullptr);
  }
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerError)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  if (!IsUwp)
  {
    EXPECT_NE(listener, nullptr);

    listener(Logger::Level::Error, "message");
    EXPECT_NE(buffer.str().find("ERROR : message"), std::string::npos);
    std::cerr.rdbuf(old);
  }
  else
  {
    EXPECT_EQ(listener, nullptr);
  }
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerWarning)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  if (!IsUwp)
  {
    EXPECT_NE(listener, nullptr);

    listener(Logger::Level::Warning, "message");
    EXPECT_NE(buffer.str().find("WARN  : message"), std::string::npos);
    std::cerr.rdbuf(old);
  }
  else
  {
    EXPECT_EQ(listener, nullptr);
  }
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerInformational)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  if (!IsUwp)
  {
    EXPECT_NE(listener, nullptr);

    listener(Logger::Level::Informational, "message");
    EXPECT_NE(buffer.str().find("INFO  : message"), std::string::npos);
    std::cerr.rdbuf(old);
  }
  else
  {
    EXPECT_EQ(listener, nullptr);
  }
}

TEST_F(EnvironmentLogLevelListenerTest, GetLogListenerUnknown)
{
  EnvironmentLogLevelListener::SetInitialized(false);
  SetLogLevel("verbose");

  std::stringstream buffer;
  std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());

  std::string text = buffer.str(); // text will now contain "Bla\n"
  auto listener = EnvironmentLogLevelListener::GetLogListener();

  if (!IsUwp)
  {
    EXPECT_NE(listener, nullptr);

    listener(static_cast<Logger::Level>(42), "message");
    EXPECT_NE(buffer.str().find("????? : message"), std::string::npos);
    std::cerr.rdbuf(old);
  }
  else
  {
    EXPECT_EQ(listener, nullptr);
  }
}
