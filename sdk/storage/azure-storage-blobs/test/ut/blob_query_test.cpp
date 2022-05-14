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

  const std::vector<uint8_t> ParquetQueryTestData = Core::Convert::Base64Decode(
      "UEFSMRUAFewBFewBLBUaFQAVBhUIAAACAAAAGgFkAAAAAAAAAGUAAAAAAAAAZgAAAAAAAABnAAAAAAAAAGgAAAAAAAAA"
      "aQAAAAAAAABqAAAAAAAAAGsAAAAAAAAAbAAAAAAAAABtAAAAAAAAAG4AAAAAAAAAbwAAAAAAAABwAAAAAAAAAAAAAAAA"
      "AAAAFQAVxAIVxAIsFRoVABUGFQgAAAIAAAAaAQcAAABvcmFuZ2VzBQAAAGxpbWVzBwAAAGJlcnJpZXMGAAAAYXBwbGVz"
      "CwAAAGNsZW1lbnRpbmVzBgAAAGdyYXBlcwYAAABsZW1vbnMFAAAAcGVhcnMIAAAAY2hlcnJpZXMHAAAAY29jb251dAcA"
      "AABiYW5hbmFzBwAAAHBlYWNoZXMOAAAAc2Fwb3RlLCJtYW1leSIAAAAAAAAAABUAFewBFewBLBUaFQAVBhUIAAACAAAA"
      "GgFkAAAAAAAAADIAAAAAAAAAxwAAAAAAAABjAAAAAAAAAI8BAAAAAAAAlgAAAAAAAABFAAAAAAAAAGQAAAAAAAAAGQEA"
      "AAAAAACyAAAAAAAAACcAAAAAAAAAdQAAAAAAAAAyAAAAAAAAAAAAAAAAAAAAFQIZTEgGc2NoZW1hFQYAFQQVgAEVAhgC"
      "aWQAFQwlAhgEbmFtZSUAABUEFYABFQIYBXByaWNlABYaGRwZPCaaAhwVBBkVABkYAmlkFQAWGhaSAhaSAhkAFgg8GAhw"
      "AAAAAAAAABgIZAAAAAAAAAAWAAAZHBUAFQAVAgAAACaEBRwVDBkVABkYBG5hbWUVABYaFuoCFuoCGQAWmgI8GA5zYXBv"
      "dGUsIm1hbWV5IhgGYXBwbGVzFgAAGRwVABUAFQIAAAAmlgccFQQZFQAZGAVwcmljZRUAFhoWkgIWkgIZABaEBTwYCI8B"
      "AAAAAAAAGAgnAAAAAAAAABYAABkcFQAVABUCAAAAFo4HFhoAGRwYBnBhbmRhcxiRBXsiY29sdW1uX2luZGV4ZXMiOiBb"
      "eyJmaWVsZF9uYW1lIjogbnVsbCwgIm1ldGFkYXRhIjogbnVsbCwgIm5hbWUiOiBudWxsLCAibnVtcHlfdHlwZSI6ICJv"
      "YmplY3QiLCAicGFuZGFzX3R5cGUiOiAibWl4ZWQtaW50ZWdlciJ9XSwgImNvbHVtbnMiOiBbeyJmaWVsZF9uYW1lIjog"
      "ImlkIiwgIm1ldGFkYXRhIjogbnVsbCwgIm5hbWUiOiAiaWQiLCAibnVtcHlfdHlwZSI6ICJpbnQ2NCIsICJwYW5kYXNf"
      "dHlwZSI6ICJpbnQ2NCJ9LCB7ImZpZWxkX25hbWUiOiAibmFtZSIsICJtZXRhZGF0YSI6IG51bGwsICJuYW1lIjogIm5h"
      "bWUiLCAibnVtcHlfdHlwZSI6ICJvYmplY3QiLCAicGFuZGFzX3R5cGUiOiAidW5pY29kZSJ9LCB7ImZpZWxkX25hbWUi"
      "OiAicHJpY2UiLCAibWV0YWRhdGEiOiBudWxsLCAibmFtZSI6ICJwcmljZSIsICJudW1weV90eXBlIjogImludDY0Iiwg"
      "InBhbmRhc190eXBlIjogImludDY0In1dLCAiY3JlYXRvciI6IHsibGlicmFyeSI6ICJmYXN0cGFycXVldCIsICJ2ZXJz"
      "aW9uIjogIjAuOC4xIn0sICJpbmRleF9jb2x1bW5zIjogW3sia2luZCI6ICJyYW5nZSIsICJuYW1lIjogbnVsbCwgInN0"
      "YXJ0IjogMCwgInN0ZXAiOiAxLCAic3RvcCI6IDEzfV0sICJwYW5kYXNfdmVyc2lvbiI6ICIxLjQuMiIsICJwYXJ0aXRp"
      "b25fY29sdW1ucyI6IFtdfQAYKmZhc3RwYXJxdWV0LXB5dGhvbiB2ZXJzaW9uIDAuOC4xIChidWlsZCAwKQDXAwAAUEFS"
      "MQ==");

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

  TEST_F(BlockBlobClientTest, QueryCsvInputArrowOutput)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    client.UploadFrom(
        reinterpret_cast<const uint8_t*>(CsvQueryTestData.data()), CsvQueryTestData.size());

    Blobs::QueryBlobOptions queryOptions;
    queryOptions.InputTextConfiguration
        = Blobs::BlobQueryInputTextOptions::CreateCsvTextOptions("\n", ",", "\"", "\\", true);
    std::vector<Blobs::Models::BlobQueryArrowField> fields;
    Blobs::Models::BlobQueryArrowField field;
    field.Type = Blobs::Models::BlobQueryArrowFieldType::Int64;
    field.Name = "id";
    fields.push_back(field);
    field.Type = Blobs::Models::BlobQueryArrowFieldType::String;
    field.Name = "name";
    fields.push_back(field);
    field.Type = Blobs::Models::BlobQueryArrowFieldType::Decimal;
    field.Name = "price";
    field.Precision = 10;
    field.Scale = 2;
    fields.push_back(field);
    queryOptions.OutputTextConfiguration
        = Blobs::BlobQueryOutputTextOptions::CreateArrowTextOptions(std::move(fields));
    auto queryResponse
        = client.Query("SELECT * from BlobStorage WHERE id > 101 AND price < 100;", queryOptions);

    auto data = queryResponse.Value.BodyStream->ReadToEnd();
    EXPECT_EQ(
        data,
        Core::Convert::Base64Decode(
            "/////"
            "+gAAAAQAAAAAAAKAAwABgAFAAgACgAAAAABBAAMAAAACAAIAAAABAAIAAAABAAAAAMAAACAAAAAQAAAAAQAAAC"
            "c////AAABBxAAAAAgAAAABAAAAAAAAAAFAAAAcHJpY2UAAAAIAAwABAAIAAgAAAAKAAAAAgAAANT///"
            "8AAAEFEAAAABwAAAAEAAAAAAAAAAQAAABuYW1lAAAAAAQABAAEAAAAEAAUAAgABgAHAAwAAAAQABAAAAAAAAEC"
            "EAAAABwAAAAEAAAAAAAAAAIAAABpZAAACAAMAAgABwAIAAAAAAAAAUAAAAAAAAAA//////"
            "AAAAAUAAAAAAAAAAwAGgAGAAUACAAMAAwAAAAAAwQAHAAAAAgAAAAAAAAAAAAAAAAACgAMAAAABAAIAAoAAACA"
            "AAAABAAAAAcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAQAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAAAAAAAAMA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAP/////"
            "4AAAAFAAAAAAAAAAMABYABgAFAAgADAAMAAAAAAMEABgAAACYAAAAAAAAAAAACgAYAAwABAAIAAoAAACMAAAAE"
            "AAAAAQAAAAAAAAAAAAAAAcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAAAAAAAAACAAAAAAAAAAAAAAAAA"
            "AAAAgAAAAAAAAABQAAAAAAAAAOAAAAAAAAAAfAAAAAAAAAFgAAAAAAAAAAAAAAAAAAABYAAAAAAAAAEAAAAAAA"
            "AAAAAAAAAMAAAAEAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAABnAAAAAAA"
            "AAGoAAAAAAAAAbgAAAAAAAABwAAAAAAAAAAAAAAAGAAAADAAAABMAAAAfAAAAAAAAAGFwcGxlc2xlbW9uc2Jhb"
            "mFuYXNzYXBvdGUsbWFtZXkAYwAAAAAAAAAAAAAAAAAAAEUAAAAAAAAAAAAAAAAAAAAnAAAAAAAAAAAAAAAAAAA"
            "AMgAAAAAAAAAAAAAAAAAAAA=="));
  }

  TEST_F(BlockBlobClientTest, QueryParquetInputArrowOutput)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    client.UploadFrom(ParquetQueryTestData.data(), ParquetQueryTestData.size());

    Blobs::QueryBlobOptions queryOptions;
    queryOptions.InputTextConfiguration
        = Blobs::BlobQueryInputTextOptions::CreateParquetTextOptions();
    std::vector<Blobs::Models::BlobQueryArrowField> fields;
    Blobs::Models::BlobQueryArrowField field;
    field.Type = Blobs::Models::BlobQueryArrowFieldType::Int64;
    field.Name = "id";
    fields.push_back(field);
    field.Type = Blobs::Models::BlobQueryArrowFieldType::String;
    field.Name = "name";
    fields.push_back(field);
    field.Type = Blobs::Models::BlobQueryArrowFieldType::Int64;
    field.Name = "price";
    fields.push_back(field);
    queryOptions.OutputTextConfiguration
        = Blobs::BlobQueryOutputTextOptions::CreateArrowTextOptions(std::move(fields));
    auto queryResponse
        = client.Query("SELECT * from BlobStorage WHERE id > 101 AND price < 100;", queryOptions);
    auto data = queryResponse.Value.BodyStream->ReadToEnd();
    EXPECT_EQ(
        data,
        Core::Convert::Base64Decode(
            "/////"
            "+AAAAAQAAAAAAAKAAwABgAFAAgACgAAAAABBAAMAAAACAAIAAAABAAIAAAABAAAAAMAAAB4AAAAOAAAAAQAAAC"
            "k////AAABAhAAAAAYAAAABAAAAAAAAAAFAAAAcHJpY2UAAACY////AAAAAUAAAADU////"
            "AAABBRAAAAAcAAAABAAAAAAAAAAEAAAAbmFtZQAAAAAEAAQABAAAABAAFAAIAAYABwAMAAAAEAAQAAAAAAABAh"
            "AAAAAcAAAABAAAAAAAAAACAAAAaWQAAAgADAAIAAcACAAAAAAAAAFAAAAAAAAAAP/////"
            "wAAAAFAAAAAAAAAAMABoABgAFAAgADAAMAAAAAAMEABwAAAAIAAAAAAAAAAAAAAAAAAoADAAAAAQACAAKAAAAg"
            "AAAAAQAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAEAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAADA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD/////"
            "+AAAABQAAAAAAAAADAAWAAYABQAIAAwADAAAAAADBAAYAAAAIAAAAAAAAAAAAAoAGAAMAAQACAAKAAAAjAAAAB"
            "AAAAABAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAIAAAAAAAAAAAAAAAA"
            "AAAACAAAAAAAAAAIAAAAAAAAABAAAAAAAAAABgAAAAAAAAAYAAAAAAAAAAAAAAAAAAAAGAAAAAAAAAAIAAAAAA"
            "AAAAAAAAADAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAZwAAAAAA"
            "AAAAAAAABgAAAGFwcGxlcwAAYwAAAAAAAAD/////"
            "+AAAABQAAAAAAAAADAAWAAYABQAIAAwADAAAAAADBAAYAAAAIAAAAAAAAAAAAAoAGAAMAAQACAAKAAAAjAAAAB"
            "AAAAABAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAIAAAAAAAAAAAAAAAA"
            "AAAACAAAAAAAAAAIAAAAAAAAABAAAAAAAAAABgAAAAAAAAAYAAAAAAAAAAAAAAAAAAAAGAAAAAAAAAAIAAAAAA"
            "AAAAAAAAADAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAagAAAAAA"
            "AAAAAAAABgAAAGxlbW9ucwAARQAAAAAAAAD/////"
            "+AAAABQAAAAAAAAADAAWAAYABQAIAAwADAAAAAADBAAYAAAAIAAAAAAAAAAAAAoAGAAMAAQACAAKAAAAjAAAAB"
            "AAAAABAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAIAAAAAAAAAAAAAAAA"
            "AAAACAAAAAAAAAAIAAAAAAAAABAAAAAAAAAABwAAAAAAAAAYAAAAAAAAAAAAAAAAAAAAGAAAAAAAAAAIAAAAAA"
            "AAAAAAAAADAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAbgAAAAAA"
            "AAAAAAAABwAAAGJhbmFuYXMAJwAAAAAAAAD/////"
            "+AAAABQAAAAAAAAADAAWAAYABQAIAAwADAAAAAADBAAYAAAAKAAAAAAAAAAAAAoAGAAMAAQACAAKAAAAjAAAAB"
            "AAAAABAAAAAAAAAAAAAAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAIAAAAAAAAAAAAAAAA"
            "AAAACAAAAAAAAAAIAAAAAAAAABAAAAAAAAAADgAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAIAAAAAA"
            "AAAAAAAAADAAAAAQAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAcAAAAAAA"
            "AAAAAAAADgAAAHNhcG90ZSwibWFtZXkiAAAyAAAAAAAAAP////8AAAAA"));
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

  TEST_F(BlockBlobClientTest, QueryLargeBlob_LIVEONLY_)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    constexpr size_t DataSize = 32_MB;

    int recordCounter = 0;
    std::string csvData;
    std::string jsonData;
    while (csvData.size() < DataSize)
    {
      std::string counter = std::to_string(recordCounter++);
      std::string record = RandomString(RandomInt(1, 3000));
      csvData += counter + "," + record + "\n";
      jsonData += "{\"_1\":\"" + counter + "\",\"_2\":\"" + record + "\"}\n";
    }

    client.UploadFrom(reinterpret_cast<const uint8_t*>(csvData.data()), csvData.size());

    Blobs::QueryBlobOptions queryOptions;
    queryOptions.InputTextConfiguration = Blobs::BlobQueryInputTextOptions::CreateCsvTextOptions();
    queryOptions.OutputTextConfiguration
        = Blobs::BlobQueryOutputTextOptions::CreateJsonTextOptions();
    auto queryResponse = client.Query("SELECT * FROM BlobStorage;", queryOptions);

    size_t comparePos = 0;
    std::vector<uint8_t> readBuffer(4096);
    while (true)
    {
      auto s = queryResponse.Value.BodyStream->Read(readBuffer.data(), readBuffer.size());
      if (s == 0)
      {
        break;
      }
      ASSERT_TRUE(comparePos + s <= jsonData.size());
      ASSERT_EQ(
          std::string(readBuffer.begin(), readBuffer.begin() + s), jsonData.substr(comparePos, s));
      comparePos += s;
    }
  }

}}} // namespace Azure::Storage::Test