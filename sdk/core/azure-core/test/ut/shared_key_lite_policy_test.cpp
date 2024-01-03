// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/base64.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/http/policies/shared_key_lite_policy.hpp>
#include <azure/core/credentials/shared_key_credential.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies;
namespace Azure { namespace Core { namespace Http { namespace Policies { namespace Test {

  TEST(SharedKeyCredentialLiteTest, SharedKeyCredentialLite)
  {
    const std::string accountKey = "account-key";
    std::string connectionString = "DefaultEndpointsProtocol=https;AccountName=account-name;"
                                   "AccountKey="
        + Azure::Core::Convert::Base64Encode(
                                       std::vector<uint8_t>(accountKey.begin(), accountKey.end()))
        + ";EndpointSuffix = core.windows.net ";

    std::shared_ptr<SharedKeyCredential> credential;
    auto parsedConnectionString
        = Azure::Core::Credentials::_internal::ParseConnectionString(connectionString);
    SharedKeyLitePolicy policy(parsedConnectionString.KeyCredential);

    Azure::Core::Url url("https://goqu.table.core.windows.net");
    url.SetQueryParameters({{"restype", "service"}, {"comp", "properties"}});
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
    request.SetHeader("x-ms-date", "Thu, 23 Apr 2020 09:43:37 GMT");
    auto result = policy.GetSignature(request);
    const std::string stringTest = "Thu, 23 Apr 2020 09:43:37 GMT\n/account-name/?comp=properties";
    const std::string expectedSignature
        = Azure::Core::Convert::Base64Encode(Azure::Core::Cryptography::HmacSha256Hash::HmacSha256(
        std::vector<uint8_t>(stringTest.begin(), stringTest.end()),
        std::vector<uint8_t>(accountKey.begin(), accountKey.end())));
    EXPECT_EQ(result, expectedSignature);
  }

  TEST(SharedKeyCredentialLiteTest, SharedKeyCredentialLiteNoDate)
  {
    const std::string accountKey = "account-key";
    std::string connectionString = "DefaultEndpointsProtocol=https;AccountName=account-name;"
                                   "AccountKey="
        + Azure::Core::Convert::Base64Encode(
                                       std::vector<uint8_t>(accountKey.begin(), accountKey.end()))
        + ";EndpointSuffix = core.windows.net ";

    std::shared_ptr<SharedKeyCredential> credential;
    auto parsedConnectionString
        = Azure::Core::Credentials::_internal::ParseConnectionString(connectionString);
    SharedKeyLitePolicy policy(parsedConnectionString.KeyCredential);

    Azure::Core::Url url("https://goqu.table.core.windows.net");
    url.SetQueryParameters({{"restype", "service"}, {"comp", "properties"}});
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
    EXPECT_THROW(policy.GetSignature(request), std::exception);
  }

  TEST(SharedKeyCredentialLiteTest, SharedKeyCredentialLiteNoQuery)
  {
    const std::string accountKey = "account-key";
    std::string connectionString = "DefaultEndpointsProtocol=https;AccountName=account-name;"
                                   "AccountKey="
        + Azure::Core::Convert::Base64Encode(
                                       std::vector<uint8_t>(accountKey.begin(), accountKey.end()))
        + ";EndpointSuffix = core.windows.net ";

    std::shared_ptr<SharedKeyCredential> credential;
    auto parsedConnectionString = Azure::Core::Credentials::_internal::ParseConnectionString(connectionString);
    SharedKeyLitePolicy policy(parsedConnectionString.KeyCredential);

    Azure::Core::Url url("https://goqu.table.core.windows.net");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
    request.SetHeader("x-ms-date", "Thu, 23 Apr 2020 09:43:37 GMT");
    auto result = policy.GetSignature(request);
    const std::string stringTest = "Thu, 23 Apr 2020 09:43:37 GMT\n/account-name/";
    const std::string expectedSignature
        = Azure::Core::Convert::Base64Encode(Azure::Core::Cryptography::HmacSha256Hash::HmacSha256(
        std::vector<uint8_t>(stringTest.begin(), stringTest.end()),
        std::vector<uint8_t>(accountKey.begin(), accountKey.end())));

    EXPECT_EQ(result, expectedSignature);
  }

}}}}} // namespace Azure::Core::Http::Policies::Test
