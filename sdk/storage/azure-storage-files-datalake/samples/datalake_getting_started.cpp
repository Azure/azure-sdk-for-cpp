// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <azure/storage/files/datalake.hpp>

std::string GetConnectionString()
{
  const static std::string ConnectionString = "";

  if (!ConnectionString.empty())
  {
    return ConnectionString;
  }
  const static std::string envConnectionString
      = std::getenv("AZURE_STORAGE_DATALAKE_CONNECTION_STRING");
  if (!envConnectionString.empty())
  {
    return envConnectionString;
  }
  throw std::runtime_error("Cannot find connection string.");
}

int main()
{
  using namespace Azure::Storage::Files::DataLake;

  const std::string fileSystemName = "sample-file-system";
  const std::string directoryName = "sample-directory";
  const std::string fileName = "sample-file";

  auto fileSystemClient
      = DataLakeFileSystemClient::CreateFromConnectionString(GetConnectionString(), fileSystemName);
  fileSystemClient.CreateIfNotExists();

  // Create a directory.
  auto directoryClient = fileSystemClient.GetDirectoryClient(directoryName);
  directoryClient.Create();

  // Creates a file under the directory.
  auto fileClient = directoryClient.GetFileClient(fileName);
  fileClient.Create();

  // Append/flush/read data from the client.
  // Append data
  // Initialize the string that contains the first piece of data to be appended to the file.
  std::string str1 = "Hello ";
  // Initialize the buffer that represents what contains your data to be appended, please ignore
  // how it is constructed here, since the memory copy is not efficient.
  std::string str2 = "Azure!";
  std::vector<uint8_t> buffer(str1.begin(), str1.end());

  // One way of passing in the buffer, note that the buffer is not copied.
  auto bufferStream = Azure::Core::IO::MemoryBodyStream(buffer);

  fileClient.Append(bufferStream, 0 /* Offset of the position to be appended.*/);

  // Another way of passing in the buffer, note that buffer is also not copied.
  bufferStream = Azure::Core::IO::MemoryBodyStream(
      reinterpret_cast<const uint8_t*>(str2.data()), str2.size());

  fileClient.Append(bufferStream, str1.size());

  // Flush
  fileClient.Flush(str1.size() + str2.size());

  // Read
  auto result = fileClient.Download();
  Azure::Core::Context context;
  std::vector<uint8_t> downloaded = result.Value.Body->ReadToEnd(context);
  // downloaded contains your downloaded data.
  std::cout << std::string(downloaded.begin(), downloaded.end()) << std::endl;

  return 0;
}
