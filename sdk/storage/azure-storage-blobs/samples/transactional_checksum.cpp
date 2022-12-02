// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <azure/storage/blobs.hpp>
#include <azure/storage/common/crypt.hpp>

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
  using namespace Azure::Storage::Blobs;

  const std::string containerName = "sample-container";
  const std::string blobName = "sample-blob";

  auto containerClient
      = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), containerName);
  containerClient.CreateIfNotExists();
  BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);

  // Upload 1MiB of data and verify with MD5
  std::vector<uint8_t> buffer;
  buffer.resize(1 * 1024 * 1024);
  UploadBlockBlobOptions uploadOptions;
  uploadOptions.TransactionalContentHash = Azure::Storage::ContentHash();
  uploadOptions.TransactionalContentHash.Value().Algorithm = Azure::Storage::HashAlgorithm::Md5;
  Azure::Core::Cryptography::Md5Hash md5Hash;
  md5Hash.Append(buffer.data(), buffer.size());
  uploadOptions.TransactionalContentHash.Value().Value = md5Hash.Final();
  Azure::Core::IO::MemoryBodyStream bodyStream(buffer);
  blobClient.Upload(bodyStream, uploadOptions);

  // Download the data and verify with CRC64
  DownloadBlobOptions downloadOptions;
  downloadOptions.Range = Azure::Core::Http::HttpRange(); // Have to specify a range, and the range
                                                          // cannot be larger than 4MiB
  downloadOptions.Range.Value().Offset = 0;
  downloadOptions.Range.Value().Length = buffer.size();
  downloadOptions.RangeHashAlgorithm = Azure::Storage::HashAlgorithm::Crc64;
  auto downloadResponse = blobClient.Download(downloadOptions);
  buffer = downloadResponse.Value.BodyStream->ReadToEnd();
  Azure::Storage::Crc64Hash crc64Hash;
  crc64Hash.Append(buffer.data(), buffer.size());
  if (crc64Hash.Final() != downloadResponse.Value.TransactionalContentHash.Value().Value)
  {
    std::cout << "CRC-64 mismatch" << std::endl;
  }
  else
  {
    std::cout << "CRC-64 match" << std::endl;
  }

  return 0;
}