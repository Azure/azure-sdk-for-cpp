// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/log.hpp>
#include <azure/core/logging/logging.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Internal;

TEST(Logging, Defaults)
{
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_FALSE(ShouldLog(LogLevel::Warning));
  EXPECT_FALSE(ShouldLog(LogLevel::Error));

  SetLogListener([](auto, auto) {});

  EXPECT_TRUE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  SetLogListener(nullptr);

  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_FALSE(ShouldLog(LogLevel::Warning));
  EXPECT_FALSE(ShouldLog(LogLevel::Error));
}

TEST(Logging, Levels)
{
  SetLogListener([](auto, auto) {});

  SetLogLevel(LogLevel::Verbose);
  EXPECT_TRUE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  SetLogLevel(LogLevel::Informational);
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  SetLogLevel(LogLevel::Warning);
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  SetLogLevel(LogLevel::Error);
  EXPECT_FALSE(ShouldLog(LogLevel::Verbose));
  EXPECT_FALSE(ShouldLog(LogLevel::Informational));
  EXPECT_FALSE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  SetLogLevel(LogLevel::Verbose);
  EXPECT_TRUE(ShouldLog(LogLevel::Verbose));
  EXPECT_TRUE(ShouldLog(LogLevel::Informational));
  EXPECT_TRUE(ShouldLog(LogLevel::Warning));
  EXPECT_TRUE(ShouldLog(LogLevel::Error));

  SetLogListener(nullptr);
}

TEST(Logging, Message)
{
  try
  {
    LogLevel level = LogLevel::Error;
    std::string message;

    SetLogListener([&](auto lvl, auto msg) {
      level = lvl;
      message = msg;
    });

    SetLogLevel(LogLevel::Verbose);
    {
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "Verbose");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "Informational");

      Log(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetLogLevel(LogLevel::Informational);
    {
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "Informational");

      Log(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetLogLevel(LogLevel::Warning);
    {
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetLogLevel(LogLevel::Error);
    {
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetLogLevel(LogLevel::Verbose);
    {
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "Verbose");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "Informational");

      Log(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Log(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetLogListener(nullptr);

    SetLogLevel(LogLevel::Verbose);
    {
      message = "";

      Log(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "");

      Log(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "");
    }
  }
  catch (...)
  {
    SetLogListener(nullptr);
    throw;
  }
}
