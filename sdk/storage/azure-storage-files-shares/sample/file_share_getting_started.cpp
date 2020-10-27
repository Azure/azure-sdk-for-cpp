// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include "azure/storage/files/shares.hpp"
#include "samples_common.hpp"

SAMPLE(FileShareGettingStarted, FileShareGettingStarted)
void FileShareGettingStarted()
{
  using namespace Azure::Storage::Files::Shares;

  std::string shareName = "sample-share";
  std::string fileName = "sample-file";
  std::string fileContent = "Hello Azure!";

  auto shareClient = ShareClient::CreateFromConnectionString(GetConnectionString(), shareName);
  try
  {
    shareClient.Create();
  }
  catch (std::runtime_error& e)
  {
    // The share may already exist
    std::cout << e.what() << std::endl;
  }

  FileClient fileClient = shareClient.GetFileClient(fileName);

  fileClient.UploadFrom(reinterpret_cast<const uint8_t*>(fileContent.data()), fileContent.size());

  std::map<std::string, std::string> fileMetadata = {{"key1", "value1"}, {"key2", "value2"}};
  fileClient.SetMetadata(fileMetadata);

  auto properties = *fileClient.GetProperties();
  for (auto metadata : properties.Metadata)
  {
    std::cout << metadata.first << ":" << metadata.second << std::endl;
  }
  fileContent.resize(static_cast<std::size_t>(properties.ContentLength));

  fileClient.DownloadTo(reinterpret_cast<uint8_t*>(&fileContent[0]), fileContent.size());

  std::cout << fileContent << std::endl;
}
