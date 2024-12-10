// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/files/shares.hpp>

#include <gtest/gtest.h>

/* cSpell:ignore rwsrwsrwt, rwxrwxrwx, rwSrwSrwT */

namespace Azure { namespace Storage { namespace Test {

  TEST(ShareUtilityTest, NfsFileMode)
  {
    auto testOctal = [&](const std::string& modeString) {
      auto mode = Files::Shares::Models::NfsFileMode::ParseOctalFileMode(modeString);
      EXPECT_EQ(mode.ToOctalFileMode(), modeString);
    };
    auto testSymbolic = [&](const std::string& modeString) {
      auto mode = Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode(modeString);
      EXPECT_EQ(mode.ToSymbolicFileMode(), modeString);
    };
    auto testOctalToSymbolic = [&](const std::string& octal, const std::string& symbolic) {
      auto mode = Files::Shares::Models::NfsFileMode::ParseOctalFileMode(octal);
      EXPECT_EQ(mode.ToSymbolicFileMode(), symbolic);
    };
    auto testSymbolicToOctal = [&](const std::string& symbolic, const std::string& octal) {
      auto mode = Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode(symbolic);
      EXPECT_EQ(mode.ToOctalFileMode(), octal);
    };

    // 0000
    testOctal("0000");
    testSymbolic("---------");
    testOctalToSymbolic("0000", "---------");
    testSymbolicToOctal("---------", "0000");

    // 1111
    testOctal("1111");
    testSymbolic("--x--x--x");
    testOctalToSymbolic("1111", "--x--x--t");
    testSymbolicToOctal("--x--x--t", "1111");

    // 2222
    testOctal("2222");
    testSymbolic("-w--wS-w-");
    testOctalToSymbolic("2222", "-w--wS-w-");
    testSymbolicToOctal("-w--wS-w-", "2222");

    // 3333
    testOctal("3333");
    testSymbolic("-wx-ws-wt");
    testOctalToSymbolic("3333", "-wx-ws-wt");
    testSymbolicToOctal("-wx-ws-wt", "3333");

    // 4444
    testOctal("4444");
    testSymbolic("r-Sr--r--");
    testOctalToSymbolic("4444", "r-Sr--r--");
    testSymbolicToOctal("r-Sr--r--", "4444");

    // 5555
    testOctal("5555");
    testSymbolic("r-sr-xr-t");
    testOctalToSymbolic("5555", "r-sr-xr-t");
    testSymbolicToOctal("r-sr-xr-t", "5555");

    // 6666
    testOctal("6666");
    testSymbolic("rwSrwSrw-");
    testOctalToSymbolic("6666", "rwSrwSrw-");
    testSymbolicToOctal("rwSrwSrw-", "6666");

    // 7777
    testOctal("7777");
    testSymbolic("rwsrwsrwt");
    testOctalToSymbolic("7777", "rwsrwsrwt");
    testSymbolicToOctal("rwsrwsrwt", "7777");

    // 0001
    testOctal("0001");
    testSymbolic("--------x");
    testOctalToSymbolic("0001", "--------x");
    testSymbolicToOctal("--------x", "0001");

    // 0010
    testOctal("0010");
    testSymbolic("-----x---");
    testOctalToSymbolic("0010", "-----x---");
    testSymbolicToOctal("-----x---", "0010");

    // 0100
    testOctal("0100");
    testSymbolic("--x------");
    testOctalToSymbolic("0100", "--x------");
    testSymbolicToOctal("--x------", "0100");

    // 0124
    testOctal("0124");
    testSymbolic("--x-w-r--");
    testOctalToSymbolic("0124", "--x-w-r--");
    testSymbolicToOctal("--x-w-r--", "0124");

    // 0777
    testOctal("0777");
    testSymbolic("rwxrwxrwx");
    testOctalToSymbolic("0777", "rwxrwxrwx");
    testSymbolicToOctal("rwxrwxrwx", "0777");

    // 4210
    testOctal("4210");
    testSymbolic("-wS--x---");
    testOctalToSymbolic("4210", "-wS--x---");
    testSymbolicToOctal("-wS--x---", "4210");

    // 1357
    testOctal("1357");
    testSymbolic("-wxr-xrwt");
    testOctalToSymbolic("1357", "-wxr-xrwt");
    testSymbolicToOctal("-wxr-xrwt", "1357");

    // 7654
    testOctal("7654");
    testSymbolic("rwSr-sr-T");
    testOctalToSymbolic("7654", "rwSr-sr-T");
    testSymbolicToOctal("rwSr-sr-T", "7654");

    // 7666
    testOctal("7666");
    testSymbolic("rwSrwSrwT");
    testOctalToSymbolic("7666", "rwSrwSrwT");
    testSymbolicToOctal("rwSrwSrwT", "7666");

    // Invalid
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseOctalFileMode("1239"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseOctalFileMode("9786"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseOctalFileMode("12344"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseOctalFileMode("12"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseOctalFileMode("test"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseOctalFileMode("rwSrwSrwT"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode("1234"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode("raSrwSrwT"),
        std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode("---rwxrwxrwx"),
        std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode("---rwx"), std::invalid_argument);
    EXPECT_THROW(
        Files::Shares::Models::NfsFileMode::ParseSymbolicFileMode("---test"),
        std::invalid_argument);
  }

}}} // namespace Azure::Storage::Test
