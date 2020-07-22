// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake/datalake.hpp"
#include "samples_common.hpp"
#include <iostream>

SAMPLE(DataLakeGettingStarted, DataLakeGettingStarted)
void DataLakeGettingStarted()
{
  // Please note that you can always reference test cases for advanced usages.

  using namespace Azure::Storage::Files::DataLake;

  std::string fileSystemName = "sample-file-system";
  std::string directoryName = "sample-directory";
  std::string fileName = "sample-file";

  // Initializing a ServiceClient that can then initialize the FileSystemClient or list file
  // systems.
  auto serviceClient = ServiceClient::CreateFromConnectionString(GetConnectionString());

  // List all file systems.
  std::string continuation;
  std::vector<FileSystem> fileSystems;
  do
  {
    auto response = serviceClient.ListFileSystems();
    if (response->Continuation.HasValue())
    {
      continuation = response->Continuation.GetValue();
    }
    fileSystems.insert(
        fileSystems.end(), response->Filesystems.begin(), response->Filesystems.end());
  } while (!continuation.empty());

  // Create file systems.
  auto fileSystemClient = serviceClient.GetFileSystemClient(fileSystemName);
  auto iter = std::find_if(
      fileSystems.begin(), fileSystems.end(), [&fileSystemName](const FileSystem& fileSystem) {
        return fileSystem.Name == fileSystemName;
      });
  if (iter == fileSystems.end())
  {
    fileSystemClient.Create();
  }

  // Create a directory.
  auto directoryClient = fileSystemClient.GetDirectoryClient(directoryName);
  directoryClient.Create();

  // Creates a file under the directory.
  auto fileClient = fileSystemClient.GetFileClient(directoryName + "/" + fileName);
  fileClient.Create();

  // Append/flush/read data from the client.
  // Append data
  size_t bufferSize = 64;
  std::vector<uint8_t> buffer(bufferSize);
  for (size_t i = 0; i < bufferSize; ++i)
  {
    buffer[i] = static_cast<uint8_t>(i >> 3);
  }
  auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
      Azure::Core::Http::MemoryBodyStream(buffer));

  fileClient.AppendData(bufferStream.get(), 0);

  // Flush
  fileClient.FlushData(bufferSize);

  // Read
  auto result = fileClient.Read();
  Azure::Core::Context context;
  auto downloaded = Azure::Core::Http::BodyStream::ReadToEnd(context, *(result->Body));
  // downloaded contains your downloaded data.

  // Delete file system.
  fileSystemClient.Delete();

  std::cout << "Successfully finished sample." << std::endl;
}
