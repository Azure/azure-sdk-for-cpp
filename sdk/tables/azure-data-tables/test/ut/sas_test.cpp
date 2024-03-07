// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "sas_test.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>

using namespace Azure::Data::Tables::Sas;
// cspell: words rwdxylacupitf raud bqft
namespace Azure { namespace Data { namespace Test {
  TEST(SasTest, TableSasBuilderTestAllSet)
  {
    TablesSasBuilder sasBuilder;
    sasBuilder.SetPermissions(TablesSasPermissions::All);
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
    EXPECT_FALSE(sasParts.at("sig").empty());
  }

  TEST(SasTest, TableSasBuilderTestMin)
  {
    TablesSasBuilder sasBuilder;
    sasBuilder.ExpiresOn
        = Azure::DateTime::Parse("2022-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);
    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::SharedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);
    EXPECT_FALSE(sasParts.at("sig").empty());
  }

  TEST(SasTest, AccountSasBuilderTestAllSet)
  {
    AccountSasBuilder sasBuilder;
    sasBuilder.SetPermissions(AccountSasPermissions::All);
    sasBuilder.Protocol = SasProtocol::HttpsAndHttp;
    sasBuilder.StartsOn
        = Azure::DateTime::Parse("2020-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);
    sasBuilder.ExpiresOn
        = Azure::DateTime::Parse("2022-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);
    sasBuilder.IPRange = "iprange";
    sasBuilder.EncryptionScope = "myScope";
    sasBuilder.ResourceTypes = AccountSasResource::All;
    sasBuilder.Services = AccountSasServices::All;

    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::SharedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);

    EXPECT_EQ(sasParts.at("?se"), "2022-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("ses"), "myScope");
    EXPECT_FALSE(sasParts.at("sig").empty());
    EXPECT_EQ(sasParts.at("sip"), "iprange");
    EXPECT_EQ(sasParts.at("sp"), "rwdxylacupitf");
    EXPECT_EQ(sasParts.at("spr"), "https,http");
    EXPECT_EQ(sasParts.at("srt"), "sco");
    EXPECT_EQ(sasParts.at("ss"), "bqft");
    EXPECT_EQ(sasParts.at("st"), "2020-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("sv"), "2023-08-03");
  }

  TEST(SasTest, AccountSasBuilderTestMin)
  {
    AccountSasBuilder sasBuilder;
    sasBuilder.SetPermissions(AccountSasPermissions::All);
    sasBuilder.ExpiresOn
        = Azure::DateTime::Parse("2022-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);

    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::SharedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);

    EXPECT_FALSE(sasParts.at("sig").empty());
  }
}}} // namespace Azure::Data::Test
