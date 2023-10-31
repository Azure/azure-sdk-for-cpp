// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_base.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/internal/shared_key_policy_lite.hpp>
#include <azure/storage/common/storage_credential.hpp>

namespace Azure { namespace Storage { namespace _internal {
  namespace Test {
      
  TEST(SharedKeyCredentialLiteTest, SharedKeyCredentialLite)
  {
    const std::string accountKey = "account-key";
    std::string connectionString = "DefaultEndpointsProtocol=https;AccountName=account-name;"
                                   "AccountKey="
        + Azure::Core::Convert::Base64Encode(
                                       std::vector<uint8_t>(accountKey.begin(), accountKey.end()))
        + ";EndpointSuffix = core.windows.net ";

    std::shared_ptr<StorageSharedKeyCredential> credential;
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    SharedKeyPolicyLite policy(parsedConnectionString.KeyCredential);

    Azure::Core::Url url("https://goqu.table.core.windows.net");
    url.SetQueryParameters({{"restype", "service"}, {"comp", "properties"}});
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
    request.SetHeader("x-ms-date", "Thu, 23 Apr 2020 09:43:37 GMT");
    auto result = policy.GetSignature(request);
    EXPECT_EQ(result, "tW8SGePdivpFOEJfTxikbSwjdDWkpxSTfFtqUMED3v8=");
    auto decodedResult = Azure::Core::Convert::Base64Decode(result);
    EXPECT_EQ(decodedResult.size(), 32);
  }

  TEST(SharedKeyCredentialLiteTest, SharedKeyCredentialLiteNoDate)
  {
    const std::string accountKey = "account-key";
    std::string connectionString = "DefaultEndpointsProtocol=https;AccountName=account-name;"
                                   "AccountKey="
        + Azure::Core::Convert::Base64Encode(
                                       std::vector<uint8_t>(accountKey.begin(), accountKey.end()))
        + ";EndpointSuffix = core.windows.net ";

    std::shared_ptr<StorageSharedKeyCredential> credential;
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    SharedKeyPolicyLite policy(parsedConnectionString.KeyCredential);

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

    std::shared_ptr<StorageSharedKeyCredential> credential;
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    SharedKeyPolicyLite policy(parsedConnectionString.KeyCredential);

    Azure::Core::Url url("https://goqu.table.core.windows.net");
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
    request.SetHeader("x-ms-date", "Thu, 23 Apr 2020 09:43:37 GMT");
    auto result = policy.GetSignature(request);
    EXPECT_EQ(result, "p5FCqEmJLSpbljP2SBDHn7BbTyEsS+LJ76hd/axtUnc=");
  }

}}}}// namespace Azure::Storage::_internal