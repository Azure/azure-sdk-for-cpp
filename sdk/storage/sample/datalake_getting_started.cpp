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

  try
  {
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

    // Find if the file systems already exist.
    auto fileSystemClient = serviceClient.GetFileSystemClient(fileSystemName);
    auto iter = std::find_if(
        fileSystems.begin(), fileSystems.end(), [&fileSystemName](const FileSystem& fileSystem) {
          return fileSystem.Name == fileSystemName;
        });
    // Create file systems if not exist.
    if (iter == fileSystems.end())
    {
      fileSystemClient.Create();
    }

    // Create a directory.
    auto directoryClient = fileSystemClient.GetDirectoryClient(directoryName);
    directoryClient.Create();

    // Creates a file under the directory.
    auto fileClient = directoryClient.GetFileClient(fileName);
    fileClient.Create();

    // Append/flush/read data from the client.
    // Append data
    std::string str1 = "Hello ";
    std::string str2 = "World!";
    std::vector<uint8_t> buffer(str1.begin(), str1.end());

    // One way of passing in the buffer, note that the buffer is not copied.
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(buffer);

    fileClient.AppendData(bufferStream.get(), 0 /* Offset of the position to be appended.*/);

    // Another way of passing in the buffer, note that buffer is also not copied.
    bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        reinterpret_cast<const uint8_t*>(str2.data()), str2.size());

    fileClient.AppendData(bufferStream.get(), str1.size());

    // Flush
    fileClient.FlushData(str1.size() + str2.size());

    // Read
    auto result = fileClient.Read();
    Azure::Core::Context context;
    std::vector<uint8_t> downloaded
        = Azure::Core::Http::BodyStream::ReadToEnd(context, *(result->Body));
    // downloaded contains your downloaded data.
    std::cout << "Downloaded data was:\n" + std::string(downloaded.begin(), downloaded.end())
              << std::endl;

    // Delete file system.
    fileSystemClient.Delete();

    std::cout << "Successfully finished sample." << std::endl;
  }
  catch (const Azure::Storage::StorageError& e)
  {
    // Deal with the information when storage error is met.
    std::cout << "Error encountered when sending the request." << std::endl;
    std::cout << "ErrorCode: " + e.ErrorCode << std::endl;
    std::cout << "Message: " + e.Message << std::endl;
    std::cout << "ReasonPhrase: " + e.ReasonPhrase << std::endl;
    std::cout << "RequestId: " + e.RequestId << std::endl;
  }
}
