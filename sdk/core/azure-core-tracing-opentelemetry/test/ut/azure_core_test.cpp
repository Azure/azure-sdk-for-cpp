// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
#if defined(AZ_PLATFORM_POSIX)
  // OpenSSL signals SIGPIPE when trying to clean an HTTPS closed connection.
  // End users need to decide if SIGPIPE should be ignored or not.
  signal(SIGPIPE, SIG_IGN);
#endif

  testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}
