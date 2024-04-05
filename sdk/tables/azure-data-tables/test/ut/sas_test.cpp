// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "sas_test.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>

using namespace Azure::Data::Tables::Sas;
// cspell: words rwdlau raud bqft
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
    sasBuilder.RowKeyEnd = "myRowKeyEnd";
    sasBuilder.RowKeyStart = "myRowKeyStart";
    sasBuilder.PartitionKeyStart = "myStartPartitionKey";
    sasBuilder.PartitionKeyEnd = "myEndPartitionKey";
    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::NamedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);
    EXPECT_EQ(sasParts.at("si"), "myIdentifier");
    EXPECT_EQ(sasParts.at("sp"), "raud");
    EXPECT_EQ(sasParts.at("st"), "2020-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("se"), "2022-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("sip"), "iprange");
    EXPECT_EQ(sasParts.at("spr"), "https,http");
    EXPECT_FALSE(sasParts.at("sig").empty());
    EXPECT_EQ(sasParts.at("srk"), "myRowKeyStart");
    EXPECT_EQ(sasParts.at("erk"), "myRowKeyEnd");
    EXPECT_EQ(sasParts.at("spk"), "myStartPartitionKey");
    EXPECT_EQ(sasParts.at("?epk"), "myEndPartitionKey");
  }
  TEST(SasTest, TableSasBuilderTestSomeSet)
  {
    TablesSasBuilder sasBuilder;

    sasBuilder.Protocol = SasProtocol::HttpsAndHttp;

    sasBuilder.ExpiresOn
        = Azure::DateTime::Parse("2022-03-11T11:13:52Z", Azure::DateTime::DateFormat::Rfc3339);
    sasBuilder.SetPermissions(TablesSasPermissions::Add);
    sasBuilder.TableName = "someTableName";

    std::string key = "*";
    Azure::Data::Tables::Credentials::NamedKeyCredential cred(
        "someaccount",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);
    EXPECT_EQ(sasParts.at("?se"), "2022-03-11T11:13:52Z");
    EXPECT_EQ(sasParts.at("sp"), "a");
    EXPECT_EQ(sasParts.at("spr"), "https,http");
    EXPECT_EQ(sasParts.at("tn"), "someTableName");
  }

  TEST(SasTest, TableSasBuilderTestMin)
  {
    TablesSasBuilder sasBuilder;
    sasBuilder.ExpiresOn
        = Azure::DateTime::Parse("2022-08-18T00:00:00Z", Azure::DateTime::DateFormat::Rfc3339);
    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::NamedKeyCredential cred(
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
    sasBuilder.ResourceTypes = AccountSasResourceType::All;
    sasBuilder.Services = AccountSasServices::All;

    std::string key = "accountKey";
    Azure::Data::Tables::Credentials::NamedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);

    EXPECT_EQ(sasParts.at("?se"), "2022-08-18T00:00:00Z");
    EXPECT_EQ(sasParts.at("ses"), "myScope");
    EXPECT_FALSE(sasParts.at("sig").empty());
    EXPECT_EQ(sasParts.at("sip"), "iprange");
    EXPECT_EQ(sasParts.at("sp"), "rwdlau");
    EXPECT_EQ(sasParts.at("spr"), "https,http");
    EXPECT_EQ(sasParts.at("srt"), "sco");
    EXPECT_EQ(sasParts.at("ss"), "t");
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
    Azure::Data::Tables::Credentials::NamedKeyCredential cred(
        "accountName",
        Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(key.begin(), key.end())));
    auto sasToken = sasBuilder.GenerateSasToken(cred);
    auto sasParts = SasTest::ParseQueryParameters(sasToken);

    EXPECT_FALSE(sasParts.at("sig").empty());
  }
}}} // namespace Azure::Data::Test
