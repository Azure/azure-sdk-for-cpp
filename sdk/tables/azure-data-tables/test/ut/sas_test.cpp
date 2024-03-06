// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "sas_test.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>

using namespace Azure::Data::Tables::Sas;

namespace Azure { namespace Data { namespace Test {
  TEST(SasTest, TableSasBuilderTest)
  {
    TableSasBuilder sasBuilder;
    sasBuilder.SetPermissions(TableSasPermissions::All);
    sasBuilder.Protocol = SasProtocol::HttpsAndHttp;
    sasBuilder.StartsOn
        = Azure::DateTime::Parse("2020-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);
    sasBuilder.ExpiresOn
        = Azure::DateTime::Parse("2022-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);
    sasBuilder.Identifier = "myIdentifier";
    sasBuilder.IPRange = "iprange";
    sasBuilder.TableName = "myTableName";
    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::SharedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);
    EXPECT_EQ(sasParts.at("si"), "myIdentifier");
    EXPECT_EQ(sasParts.at("sp"), "raud");
    EXPECT_EQ(sasParts.at("st"), "2020-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("?se"), "2022-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("sip"), "iprange");
    EXPECT_EQ(sasParts.at("spr"), "https,http");
  }

}}} // namespace Azure::Data::Test