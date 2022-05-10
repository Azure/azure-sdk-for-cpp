// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <get_env.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <azure/storage/files/shares.hpp>

std::string GetConnectionString()
{
  const static std::string ConnectionString = "";

  if (!ConnectionString.empty())
  {
    return ConnectionString;
  }
  const static std::string envConnectionString = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
  if (!envConnectionString.empty())
  {
    return envConnectionString;
  }
  throw std::runtime_error("Cannot find connection string.");
}

int main()
{
  using namespace Azure::Storage::Files::Shares;

  const std::string shareName = "sample-share";
  const std::string fileName = "sample-file";
  const std::string fileContent = "Hello Azure!";

  auto shareClient = ShareClient::CreateFromConnectionString(GetConnectionString(), shareName);
  shareClient.CreateIfNotExists();

  ShareFileClient fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);

  std::vector<uint8_t> buffer(fileContent.begin(), fileContent.end());
  fileClient.UploadFrom(buffer.data(), buffer.size());

  Azure::Storage::Metadata fileMetadata = {{"key1", "value1"}, {"key2", "value2"}};
  fileClient.SetMetadata(fileMetadata);

  auto properties = fileClient.GetProperties().Value;
  for (auto metadata : properties.Metadata)
  {
    std::cout << metadata.first << ":" << metadata.second << std::endl;
  }
  // We know file size is small, so it's safe to cast here.
  buffer.resize(static_cast<size_t>(properties.FileSize));

  fileClient.DownloadTo(buffer.data(), buffer.size());

  std::cout << std::string(buffer.begin(), buffer.end()) << std::endl;

  return 0;
}
