//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <iostream>

#include <azure/storage/blobs.hpp>

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

  const std::string blobContent =
      R"json(
{"id": 100, "name": "oranges", "price": 100}
{"id": 101, "name": "limes", "price": 50}
{"id": 102, "name": "berries", "price": 199}
{"id": 103, "name": "apples", "price": 99}
{"id": 104, "name": "clementines", "price": 399}
{"id": 105, "name": "grapes", "price": 150}
{"id": 106, "name": "lemons", "price": 69}
{"id": 107, "name": "pears", "price": 100}
{"id": 108, "name": "cherries", "price": 281}
{"id": 109, "name": "coconut", "price": 178}
{"id": 110, "name": "bananas", "price": 39}
{"id": 111, "name": "peaches", "price": 117}
)json";

  std::vector<uint8_t> buffer(blobContent.begin(), blobContent.end());
  blobClient.UploadFrom(buffer.data(), buffer.size());

  QueryBlobOptions queryOptions;
  // input can be one of csv, json, parquet
  queryOptions.InputTextConfiguration = BlobQueryInputTextOptions::CreateJsonTextOptions();
  // output can be one of csv, json, arrow, parquet
  queryOptions.OutputTextConfiguration = BlobQueryOutputTextOptions::CreateCsvTextOptions();
  auto queryResponse
      = blobClient.Query("SELECT * from BlobStorage WHERE id > 101 AND price < 100;", queryOptions);

  auto data = queryResponse.Value.BodyStream->ReadToEnd();
  std::cout << std::string(data.begin(), data.end());
  /* The output is:
   * 103,apples,99
   * 106,lemons,69
   * 110,bananas,39
   */

  return 0;
}
