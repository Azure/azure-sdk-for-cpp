// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/log.hpp>
#include <gtest/gtest.h>

using Azure::Core::Logger;
using Azure::Core::LogLevel;
using Azure::Core::Internal::Log;
using Azure::Core::Internal::ShouldLog;

TEST(Logging, Defaults)
{
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_FALSE(ShouldLog(LogLevel::Warning));
  EXPECT_FALSE(ShouldLog(LogLevel::Error));

  Logger::SetListener([](auto, auto) {});

  EXPECT_TRUE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  Logger::SetListener(nullptr);

  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_FALSE(ShouldLog(LogLevel::Warning));
  EXPECT_FALSE(ShouldLog(LogLevel::Error));
}

TEST(Logging, Levels)
{
  Logger::SetListener([](auto, auto) {});

  Logger::SetLevel(LogLevel::Verbose);
  EXPECT_TRUE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  Logger::SetLevel(LogLevel::Informational);
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  Logger::SetLevel(LogLevel::Warning);
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  Logger::SetLevel(LogLevel::Error);
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_FALSE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  Logger::SetLevel(LogLevel::Verbose);
  EXPECT_TRUE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  Logger::SetListener(nullptr);
}

TEST(Logging, Message)
{
  try
  {
    LogLevel level = LogLevel::Error;
    std::string message;

    Logger::SetListener([&](auto lvl, auto msg) {
      level = lvl;
      message = msg;
    });

    Logger::SetLevel(LogLevel::Verbose);
    {
      level = LogLevel::Error;
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(level, LogLevel::Verbose);
      EXPECT_EQ(message, "Verbose");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(level, LogLevel::Informational);
      EXPECT_EQ(message, "Informational");

      Log(LogLevel::Warning, "Warning");
      EXPECT_EQ(level, LogLevel::Warning);
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Error, "Error");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(LogLevel::Informational);
    {
      level = LogLevel::Error;
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(level, LogLevel::Informational);
      EXPECT_EQ(message, "Informational");

      Log(LogLevel::Warning, "Warning");
      EXPECT_EQ(level, LogLevel::Warning);
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Error, "Error");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(LogLevel::Warning);
    {
      level = LogLevel::Error;
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Warning, "Warning");
      EXPECT_EQ(level, LogLevel::Warning);
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Error, "Error");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(LogLevel::Error);
    {
      level = LogLevel::Error;
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Warning, "Warning");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      level = LogLevel::Verbose;

      Log(LogLevel::Error, "Error");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "Error");
    }

    // Verify that we can switch back to Verbose
    Logger::SetLevel(LogLevel::Verbose);
    {
      level = LogLevel::Error;
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(level, LogLevel::Verbose);
      EXPECT_EQ(message, "Verbose");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(level, LogLevel::Informational);
      EXPECT_EQ(message, "Informational");

      Log(LogLevel::Warning, "Warning");
      EXPECT_EQ(level, LogLevel::Warning);
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Error, "Error");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetListener(nullptr);

    Logger::SetLevel(LogLevel::Verbose);
    {
      level = LogLevel::Error;
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      Log(LogLevel::Warning, "Warning");
      EXPECT_EQ(level, LogLevel::Error);
      EXPECT_EQ(message, "");

      level = LogLevel::Verbose;

      Log(LogLevel::Error, "Error");
      EXPECT_EQ(level, LogLevel::Verbose);
      EXPECT_EQ(message, "");
    }
  }
  catch (...)
  {
    Logger::SetListener(nullptr);
    throw;
  }
}
