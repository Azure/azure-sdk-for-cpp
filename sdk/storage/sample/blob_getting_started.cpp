// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob.hpp"
#include "azure.hpp"

int main()
{
  std::string sasQuery = "";

  std::string accountUrl = "https://katcpptrack2test.blob.core.windows.net/";
  using namespace Azure::Storage::Blobs;

  BlobRestClient::Container::CreateOptions options;

  options.AccessType = BlobRestClient::Container::PublicAccessType::Anonymous;

  std::string containerName = "testcontainer4";
  std::string blobName = "blobName";
  //Push for base64 encoder/decoder
  std::string blockId1 = "MDAwMDAwMQ==";
  std::string blockId2 = "MDAwMDAwMg==";

  std::string containerUrl = accountUrl + containerName + '/' + sasQuery;
  std::string blobUrl = accountUrl + containerName + '/' + blobName + sasQuery;

  auto result = BlobRestClient::Container::Create(containerUrl, options);

  BlobRestClient::BlockBlob::StageBlockOptions stageBlockOptions1;
  BlobRestClient::BlockBlob::StageBlockOptions stageBlockOptions2;

  uint8_t size = 100;
  uint8_t* buffer = new uint8_t[size];
  for (unsigned i = 0; i < size; ++i)
  {
    buffer[i] = i;
  }
  azure::core::http::BodyBuffer bodyBuffer1(buffer, size);
  azure::core::http::BodyBuffer bodyBuffer2(buffer, size);
  stageBlockOptions1.BlockId = blockId1;
  stageBlockOptions1.BodyBuffer = &bodyBuffer1;
  stageBlockOptions2.BlockId = blockId2;
  stageBlockOptions2.BodyBuffer = &bodyBuffer2;

  auto commitBlockResult = BlobRestClient::BlockBlob::StageBlock(blobUrl, stageBlockOptions1);

  commitBlockResult = BlobRestClient::BlockBlob::StageBlock(blobUrl, stageBlockOptions2);

  std::vector<std::pair<BlobRestClient::BlockBlob::BlockType, std::string>> blockList;
  blockList.push_back(std::make_pair(BlobRestClient::BlockBlob::BlockType::Uncommitted, blockId1));
  blockList.push_back(std::make_pair(BlobRestClient::BlockBlob::BlockType::Uncommitted, blockId2));

  BlobRestClient::BlockBlob::StageBlockListOptions stageBlockListOptions;
  stageBlockListOptions.BlockList = blockList;
  auto commitBlockListResult
      = BlobRestClient::BlockBlob::StageBlockList(blobUrl, stageBlockListOptions);

      system("pause");

  delete[] buffer;
}