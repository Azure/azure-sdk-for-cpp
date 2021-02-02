// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/log.hpp>
#include <azure/core/logging/logging.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Internal;

TEST(Logging, Defaults)
{
  EXPECT_FALSE(ShouldWrite(LogLevel::Verbose));
  EXPECT_FALSE(ShouldWrite(LogLevel::Informational));
  EXPECT_FALSE(ShouldWrite(LogLevel::Warning));
  EXPECT_FALSE(ShouldWrite(LogLevel::Error));

  SetLogListener([](auto, auto) {});

  EXPECT_TRUE(ShouldWrite(LogLevel::Verbose));
  EXPECT_TRUE(ShouldWrite(LogLevel::Informational));
  EXPECT_TRUE(ShouldWrite(LogLevel::Warning));
  EXPECT_TRUE(ShouldWrite(LogLevel::Error));

  SetLogListener(nullptr);

  EXPECT_FALSE(ShouldWrite(LogLevel::Verbose));
  EXPECT_FALSE(ShouldWrite(LogLevel::Informational));
  EXPECT_FALSE(ShouldWrite(LogLevel::Warning));
  EXPECT_FALSE(ShouldWrite(LogLevel::Error));
}

TEST(Logging, Levels)
{
  SetLogListener([](auto, auto) {});

  SetMaximumLogLevel(LogLevel::Verbose);
  EXPECT_TRUE(ShouldWrite(LogLevel::Verbose));
  EXPECT_TRUE(ShouldWrite(LogLevel::Informational));
  EXPECT_TRUE(ShouldWrite(LogLevel::Warning));
  EXPECT_TRUE(ShouldWrite(LogLevel::Error));

  SetMaximumLogLevel(LogLevel::Informational);
  EXPECT_FALSE(ShouldWrite(LogLevel::Verbose));
  EXPECT_TRUE(ShouldWrite(LogLevel::Informational));
  EXPECT_TRUE(ShouldWrite(LogLevel::Warning));
  EXPECT_TRUE(ShouldWrite(LogLevel::Error));

  SetMaximumLogLevel(LogLevel::Warning);
  EXPECT_FALSE(ShouldWrite(LogLevel::Verbose));
  EXPECT_FALSE(ShouldWrite(LogLevel::Informational));
  EXPECT_TRUE(ShouldWrite(LogLevel::Warning));
  EXPECT_TRUE(ShouldWrite(LogLevel::Error));

  SetMaximumLogLevel(LogLevel::Error);
  EXPECT_FALSE(ShouldWrite(LogLevel::Verbose));
  EXPECT_FALSE(ShouldWrite(LogLevel::Informational));
  EXPECT_FALSE(ShouldWrite(LogLevel::Warning));
  EXPECT_TRUE(ShouldWrite(LogLevel::Error));

  SetMaximumLogLevel(LogLevel::Verbose);
  EXPECT_TRUE(ShouldWrite(LogLevel::Verbose));
  EXPECT_TRUE(ShouldWrite(LogLevel::Informational));
  EXPECT_TRUE(ShouldWrite(LogLevel::Warning));
  EXPECT_TRUE(ShouldWrite(LogLevel::Error));

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

    SetMaximumLogLevel(LogLevel::Verbose);
    {
      message = "";

      Write(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "Verbose");

      Write(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "Informational");

      Write(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Write(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetMaximumLogLevel(LogLevel::Informational);
    {
      message = "";

      Write(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "Informational");

      Write(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Write(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetMaximumLogLevel(LogLevel::Warning);
    {
      message = "";

      Write(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Write(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetMaximumLogLevel(LogLevel::Error);
    {
      message = "";

      Write(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetMaximumLogLevel(LogLevel::Verbose);
    {
      message = "";

      Write(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "Verbose");

      Write(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "Informational");

      Write(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "Warning");

      Write(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "Error");
    }

    SetLogListener(nullptr);

    SetMaximumLogLevel(LogLevel::Verbose);
    {
      message = "";

      Write(LogLevel::Verbose, "Verbose");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Informational");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Warning");
      EXPECT_EQ(message, "");

      Write(LogLevel::Informational, "Error");
      EXPECT_EQ(message, "");
    }
  }
  catch (...)
  {
    SetLogListener(nullptr);
    throw;
  }
}
