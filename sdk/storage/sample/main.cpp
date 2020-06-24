// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "samples_common.hpp"

#include <cstdio>
#include <stdexcept>

const std::string& GetConnectionString()
{
  const static std::string c_ConnectionString = "";

  if (!c_ConnectionString.empty())
  {
    return c_ConnectionString;
  }
  const static std::string envConnectionString = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
  if (!envConnectionString.empty())
  {
    return envConnectionString;
  }
  throw std::runtime_error("Cannot find connection string");
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <sample name>\n", argv[0]);
  }
  else if (std::string(argv[1]) == "All")
  {
    for (auto sample : Sample::samples())
    {
      auto func = sample.second;
      func();
    }
  }
  else
  {
    auto ite = Sample::samples().find(argv[1]);
    if (ite == Sample::samples().end())
    {
      printf("Cannot find sample %s\n", argv[1]);
    }
    else
    {
      auto func = ite->second;
      func();
      return 0;
    }
  }

  printf("\nAvailable sample names:\n    All\n");
  for (const auto& i : Sample::samples())
  {
    printf("    %s\n", i.first.data());
  }
  return 1;
}
