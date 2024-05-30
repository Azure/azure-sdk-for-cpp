// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/internal/io/file_helpers.hpp>

#include <fstream>

#include <gtest/gtest.h>

using namespace Azure::Core::IO::_internal;

TEST(FileHelpers, GetFileSize_Basic)
{
  {
    std::ofstream outfile("testFileWithData.txt");
    outfile << "123";
    outfile.close();
  }

  EXPECT_EQ(3, FileHelpers::GetFileSize("testFileWithData.txt"));
}

TEST(FileHelpers, GetFileSize_Empty)
{
  {
    std::ofstream outfile("emptyTestFile.txt");
    outfile.close();
  }

  EXPECT_EQ(0, FileHelpers::GetFileSize("emptyTestFile.txt"));
}

TEST(FileHelpers, GetFileSize_NonExistent)
{
  EXPECT_THROW(FileHelpers::GetFileSize(""), std::runtime_error);
  EXPECT_THROW(FileHelpers::GetFileSize("nonexistentfile"), std::runtime_error);
  EXPECT_THROW(FileHelpers::GetFileSize("nonexistentfile.txt"), std::runtime_error);
}

TEST(FileHelpers, CreateFileDirectory_Basic)
{
  EXPECT_TRUE(FileHelpers::CreateFileDirectory("newTestDirectory"));
  EXPECT_FALSE(FileHelpers::CreateFileDirectory("newTestDirectory"));

  EXPECT_TRUE(FileHelpers::CreateFileDirectory("newTestDirectory/subdirectory"));
  EXPECT_FALSE(FileHelpers::CreateFileDirectory("newTestDirectory/subdirectory"));

  EXPECT_TRUE(FileHelpers::CreateFileDirectory("newTestDirectory\\anotherSubdirectory"));
  EXPECT_FALSE(FileHelpers::CreateFileDirectory("newTestDirectory\\anotherSubdirectory"));
}

TEST(FileHelpers, CreateFileDirectory_NonExistent)
{
  EXPECT_THROW(FileHelpers::CreateFileDirectory(""), std::runtime_error);
  EXPECT_THROW(
      FileHelpers::CreateFileDirectory("nonexistentdirectory/subdirectory"), std::runtime_error);
  EXPECT_THROW(
      FileHelpers::CreateFileDirectory("nonexistentdirectory\\subdirectory"), std::runtime_error);
}
