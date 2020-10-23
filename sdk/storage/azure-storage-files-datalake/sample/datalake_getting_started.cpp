// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include "azure/storage/files/datalake.hpp"
#include "samples_common.hpp"

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
  // Initializing a FileSystemClient that can then initialize the PathClient, FileClient,
  // DirectoryClient.
  auto fileSystemClient
      = FileSystemClient::CreateFromConnectionString(GetConnectionString(), fileSystemName);

  try
  {
    // Create file systems and ignore the already exist error.
    try
    {
      fileSystemClient.Create();
    }
    catch (Azure::Storage::StorageError& e)
    {
      if (e.ErrorCode != "ContainerAlreadyExists")
      {
        throw;
      }
      else
      {
        std::cout << "ErrorCode: " + e.ErrorCode << std::endl;
        std::cout << "ReasonPhrase: " + e.ReasonPhrase << std::endl;
      }
    }

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
    std::string str2 = "World!";
    std::vector<uint8_t> buffer(str1.begin(), str1.end());

    // One way of passing in the buffer, note that the buffer is not copied.
    auto bufferStream = Azure::Core::Http::MemoryBodyStream(buffer);

    fileClient.AppendData(&bufferStream, 0 /* Offset of the position to be appended.*/);

    // Another way of passing in the buffer, note that buffer is also not copied.
    bufferStream = Azure::Core::Http::MemoryBodyStream(
        reinterpret_cast<const uint8_t*>(str2.data()), str2.size());

    fileClient.AppendData(&bufferStream, str1.size());

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

    // List all file systems.
    std::string continuation;
    std::vector<FileSystem> fileSystems;
    do
    {
      auto response = serviceClient.ListFileSystemsSegement();
      if (response->ContinuationToken.HasValue())
      {
        continuation = response->ContinuationToken.GetValue();
      }
      fileSystems.insert(
          fileSystems.end(), response->Filesystems.begin(), response->Filesystems.end());
    } while (!continuation.empty());

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
