// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include <future>
#include <random>
#include <vector>

namespace Azure { namespace Storage { namespace Test {

  const std::string JsonQueryTestData =
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
{"id": 112, "name": "sapote,\"mamey", "price": 50}
)json";

  const std::string CsvQueryTestData = R"csv(
id,name,price
100,oranges,100
101,limes,50
102,berries,199
103,apples,99
104,clementines,399
105,grapes,150
106,lemons,69
107,pears,100
108,cherries,281
109,coconut,178
110,bananas,39
111,peaches,117
112,sapote\,mamey,50
)csv";

  TEST_F(BlockBlobClientTest, QueryJsonInputCsvOutput)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    client.UploadFrom(
        reinterpret_cast<const uint8_t*>(JsonQueryTestData.data()), JsonQueryTestData.size());

    Blobs::QueryBlobOptions queryOptions;
    queryOptions.InputTextConfiguration = Blobs::BlobQueryInputTextOptions::CreateJsonTextOptions();

    {
      queryOptions.OutputTextConfiguration
          = Blobs::BlobQueryOutputTextOptions::CreateCsvTextOptions();
      auto queryResponse
          = client.Query("SELECT * from BlobStorage WHERE id > 101 AND price < 100;", queryOptions);
      auto data = queryResponse.Value.BodyStream->ReadToEnd();
      EXPECT_EQ(
          std::string(data.begin(), data.end()),
          R"csv(103,apples,99
106,lemons,69
110,bananas,39
112,"sapote,""mamey",50
)csv");
    }

    {
      queryOptions.OutputTextConfiguration
          = Blobs::BlobQueryOutputTextOptions::CreateCsvTextOptions("|", ".", "[", "\\", true);
      auto queryResponse
          = client.Query("SELECT * from BlobStorage WHERE id > 101 AND price < 100;", queryOptions);

      auto data = queryResponse.Value.BodyStream->ReadToEnd();
      EXPECT_EQ(
          std::string(data.begin(), data.end()),
          R"csv(103.apples.99|106.lemons.69|110.bananas.39|112.sapote,"mamey.50|)csv");
    }
  }

  TEST_F(BlockBlobClientTest, QueryCsvInputJsonOutput)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    client.UploadFrom(
        reinterpret_cast<const uint8_t*>(CsvQueryTestData.data()), CsvQueryTestData.size());

    Blobs::QueryBlobOptions queryOptions;
    queryOptions.InputTextConfiguration
        = Blobs::BlobQueryInputTextOptions::CreateCsvTextOptions("\n", ",", "\"", "\\", true);
    queryOptions.OutputTextConfiguration
        = Blobs::BlobQueryOutputTextOptions::CreateJsonTextOptions("|");
    auto queryResponse
        = client.Query("SELECT * from BlobStorage WHERE id > 101 AND price < 100;", queryOptions);

    auto data = queryResponse.Value.BodyStream->ReadToEnd();
    EXPECT_EQ(
        std::string(data.begin(), data.end()),
        R"json({"id":"103","name":"apples","price":"99"}|{"id":"106","name":"lemons","price":"69"}|{"id":"110","name":"bananas","price":"39"}|{"id":"112","name":"sapote,mamey","price":"50"}|)json");
  }

  TEST_F(BlockBlobClientTest, QueryWithError)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    const std::string mailformedData =
        R"json(
{"id": 100, "name": "oranges", "price": 100}
{"id": 101, "name": "limes", "price": "aa"}
{"id": 102, "name": "berries", "price": 199}
{"id": 103, "name": "apples", "price": "bb"}
{"id": 104, "name": "clementines", "price": 399}
xx
)json";
    client.UploadFrom(
        reinterpret_cast<const uint8_t*>(mailformedData.data()), mailformedData.size());

    Blobs::QueryBlobOptions queryOptions;
    queryOptions.InputTextConfiguration = Blobs::BlobQueryInputTextOptions::CreateJsonTextOptions();
    queryOptions.OutputTextConfiguration
        = Blobs::BlobQueryOutputTextOptions::CreateJsonTextOptions();
    auto queryResponse = client.Query("SELECT * FROM BlobStorage WHERE price > 0;", queryOptions);

    auto data = queryResponse.Value.BodyStream->ReadToEnd();
  }

  TEST_F(BlockBlobClientTest, QueryDefaultInputOutput)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    const std::string csvData = "100,oranges,100";
    client.UploadFrom(reinterpret_cast<const uint8_t*>(csvData.data()), csvData.size());
    auto queryResponse = client.Query("SELECT * from BlobStorage;");

    auto data = queryResponse.Value.BodyStream->ReadToEnd();
  }
}}} // namespace Azure::Storage::Test