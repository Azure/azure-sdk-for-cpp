// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/log.hpp>
#include <gtest/gtest.h>

using Azure::Core::Logger;
using Azure::Core::Internal::Log;
using Azure::Core::Internal::ShouldLog;

TEST(Logger, Defaults)
{
  EXPECT_FALSE(ShouldLog(Logger::Level::Verbose));
  EXPECT_FALSE(ShouldLog(Logger::Level::Informational));
  EXPECT_FALSE(ShouldLog(Logger::Level::Warning));
  EXPECT_FALSE(ShouldLog(Logger::Level::Error));

  Logger::SetListener([](auto, auto) {});

  EXPECT_FALSE(ShouldLog(Logger::Level::Verbose));
  EXPECT_FALSE(ShouldLog(Logger::Level::Informational));
  EXPECT_TRUE(ShouldLog(Logger::Level::Warning));
  EXPECT_TRUE(ShouldLog(Logger::Level::Error));

  Logger::SetListener(nullptr);

  EXPECT_FALSE(ShouldLog(Logger::Level::Verbose));
  EXPECT_FALSE(ShouldLog(Logger::Level::Informational));
  EXPECT_FALSE(ShouldLog(Logger::Level::Warning));
  EXPECT_FALSE(ShouldLog(Logger::Level::Error));
}

TEST(Logger, Levels)
{
  Logger::SetListener([](auto, auto) {});

  Logger::SetLevel(Logger::Level::Verbose);
  EXPECT_TRUE(ShouldLog(Logger::Level::Verbose));
  EXPECT_TRUE(ShouldLog(Logger::Level::Informational));
  EXPECT_TRUE(ShouldLog(Logger::Level::Warning));
  EXPECT_TRUE(ShouldLog(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Informational);
  EXPECT_FALSE(ShouldLog(Logger::Level::Verbose));
  EXPECT_TRUE(ShouldLog(Logger::Level::Informational));
  EXPECT_TRUE(ShouldLog(Logger::Level::Warning));
  EXPECT_TRUE(ShouldLog(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Warning);
  EXPECT_FALSE(ShouldLog(Logger::Level::Verbose));
  EXPECT_FALSE(ShouldLog(Logger::Level::Informational));
  EXPECT_TRUE(ShouldLog(Logger::Level::Warning));
  EXPECT_TRUE(ShouldLog(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Error);
  EXPECT_FALSE(ShouldLog(Logger::Level::Verbose));
  EXPECT_FALSE(ShouldLog(Logger::Level::Informational));
  EXPECT_FALSE(ShouldLog(Logger::Level::Warning));
  EXPECT_TRUE(ShouldLog(Logger::Level::Error));

  Logger::SetLevel(Logger::Level::Verbose);
  EXPECT_TRUE(ShouldLog(Logger::Level::Verbose));
  EXPECT_TRUE(ShouldLog(Logger::Level::Informational));
  EXPECT_TRUE(ShouldLog(Logger::Level::Warning));
  EXPECT_TRUE(ShouldLog(Logger::Level::Error));

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

      Log(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Verbose);
      EXPECT_EQ(message, "Verbose");

      Log(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Informational);
      EXPECT_EQ(message, "Informational");

      Log(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(Logger::Level::Informational);
    {
      level = Logger::Level::Error;
      message = "";

      Log(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Informational);
      EXPECT_EQ(message, "Informational");

      Log(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(Logger::Level::Warning);
    {
      level = Logger::Level::Error;
      message = "";

      Log(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetLevel(Logger::Level::Error);
    {
      level = Logger::Level::Error;
      message = "";

      Log(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      level = Logger::Level::Verbose;

      Log(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    // Verify that we can switch back to Verbose
    Logger::SetLevel(Logger::Level::Verbose);
    {
      level = Logger::Level::Error;
      message = "";

      Log(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Verbose);
      EXPECT_EQ(message, "Verbose");

      Log(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Informational);
      EXPECT_EQ(message, "Informational");

      Log(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Warning);
      EXPECT_EQ(message, "Warning");

      Log(Logger::Level::Error, "Error");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "Error");
    }

    Logger::SetListener(nullptr);

    Logger::SetLevel(Logger::Level::Verbose);
    {
      level = Logger::Level::Error;
      message = "";

      Log(Logger::Level::Verbose, "Verbose");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Informational, "Informational");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      Log(Logger::Level::Warning, "Warning");
      EXPECT_EQ(level, Logger::Level::Error);
      EXPECT_EQ(message, "");

      level = Logger::Level::Verbose;

      Log(Logger::Level::Error, "Error");
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
