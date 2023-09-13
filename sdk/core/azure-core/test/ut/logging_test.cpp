// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/internal/diagnostics/log.hpp>

#include <iomanip>

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

TEST(Logger, LoggerStream)
{
  Logger::Level level = Logger::Level::Error;
  std::string message;

  Logger::SetListener([&](auto lvl, auto msg) {
    level = lvl;
    message = msg;
  });

  Logger::SetLevel(Logger::Level::Verbose);
  {
    Log::Stream(Logger::Level::Verbose) << "Verbose";
    EXPECT_EQ(message, "Verbose");
    message.clear();

    Log::Stream(Logger::Level::Informational) << "Informational" << 10;
    EXPECT_EQ(message, "Informational10");
    message.clear();

    Log::Stream(Logger::Level::Warning) << "Warning" << std::endl;
    EXPECT_EQ(message, "Warning\n");
    message.clear();

    Log::Stream(Logger::Level::Error) << "Error";
    EXPECT_EQ(message, "Error");
    message.clear();
  }

  Logger::SetLevel(Logger::Level::Informational);
  {
    Log::Stream(Logger::Level::Verbose) << "Verbose" << std::endl;
    EXPECT_EQ(message, "");
    message.clear();

    Log::Stream(Logger::Level::Informational) << "Informational";
    EXPECT_EQ(message, "Informational");
    message.clear();

    Log::Stream(Logger::Level::Warning) << "Warning";
    EXPECT_EQ(message, "Warning");
    message.clear();

    Log::Stream(Logger::Level::Error) << "Error";
    EXPECT_EQ(message, "Error");
    message.clear();
  }

  Logger::SetLevel(Logger::Level::Warning);
  {
    Log::Stream(Logger::Level::Verbose) << "Verbose";
    EXPECT_EQ(message, "");
    message.clear();

    Log::Stream(Logger::Level::Informational) << "Informational";
    EXPECT_EQ(message, "");
    message.clear();

    Log::Stream(Logger::Level::Warning) << "Warning";
    EXPECT_EQ(message, "Warning");
    message.clear();

    Log::Stream(Logger::Level::Error) << "Error";
    EXPECT_EQ(message, "Error");
    message.clear();
  }

  Logger::SetLevel(Logger::Level::Error);
  {
    Log::Stream(Logger::Level::Verbose) << "Verbose";
    EXPECT_EQ(message, "");
    message.clear();

    Log::Stream(Logger::Level::Informational) << "Informational";
    EXPECT_EQ(message, "");
    message.clear();

    Log::Stream(Logger::Level::Warning) << "Warning";
    EXPECT_EQ(message, "");
    message.clear();

    Log::Stream(Logger::Level::Error) << "Error";
    EXPECT_EQ(message, "Error");
    message.clear();
  }
}

TEST(Logger, LoggerStreamInsertion)
{
  Log::Stream(Logger::Level::Verbose) << "Verbose" << std::hex << 16 << std::endl;
  Log::Stream(Logger::Level::Verbose) << "Verbose" << std::dec << 16 << std::endl;
  Log::Stream(Logger::Level::Verbose) << "Verbose" << std::oct << 16 << std::endl;
  Log::Stream(Logger::Level::Verbose)
      << "Verbose" << std::hex << std::setw(4) << std::setfill('0') << 16 << std::endl;
  auto time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  Log::Stream(Logger::Level::Verbose)
      << "Verbose" << std::put_time(localtime(&time_t), "%c") << std::endl;
}
