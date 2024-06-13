// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/internal/io/file_helpers.hpp>
#include <azure/core/uuid.hpp>

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
  auto uuid = Azure::Core::Uuid::CreateUuid();
  std::string directoryNameSuffix = uuid.ToString();

  std::string rootDirectory = "newTestDirectory-" + directoryNameSuffix;
  EXPECT_TRUE(FileHelpers::CreateFileDirectory(rootDirectory));
  EXPECT_FALSE(FileHelpers::CreateFileDirectory(rootDirectory));

  std::string subDirectory = rootDirectory + "/" + "subdirectory-" + directoryNameSuffix;
  EXPECT_TRUE(FileHelpers::CreateFileDirectory(subDirectory));
  EXPECT_FALSE(FileHelpers::CreateFileDirectory(subDirectory));

  std::string anotherSubdirectory
      = rootDirectory + "\\" + "anotherSubdirectory-" + directoryNameSuffix;
  EXPECT_TRUE(FileHelpers::CreateFileDirectory(anotherSubdirectory));
  EXPECT_FALSE(FileHelpers::CreateFileDirectory(anotherSubdirectory));
}

TEST(FileHelpers, CreateFileDirectory_NonExistent)
{
  EXPECT_THROW(FileHelpers::CreateFileDirectory(""), std::runtime_error);
  EXPECT_THROW(
      FileHelpers::CreateFileDirectory("nonexistentdirectory/subdirectory"), std::runtime_error);
  EXPECT_THROW(
      FileHelpers::CreateFileDirectory("nonexistentdirectory\\subdirectory"), std::runtime_error);
}
