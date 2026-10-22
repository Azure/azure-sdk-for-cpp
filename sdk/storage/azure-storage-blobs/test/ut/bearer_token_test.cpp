// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlockBlobClientTest, TokenCredentialWorks)
  {
    const std::string containerName = LowercaseRandomString();
    const auto containerUrl = GetBlobContainerUrl(containerName);
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName);
    auto tokenCredential = GetTestCredential();
    containerClient = Blobs::BlobContainerClient(
        containerUrl, tokenCredential, InitStorageClientOptions<Blobs::BlobClientOptions>());

    EXPECT_NO_THROW(containerClient.Create());
    EXPECT_NO_THROW(containerClient.Delete());
  }

  TEST_F(SyntheticStorageTest, BlobBearerChallengeWorks_PLAYBACKONLY_)
  {
    const std::string aadTenantId = "11111111-1111-1111-1111-111111111111";
    const std::string aadClientId = "synthetic-client";
    const std::string aadClientSecret = "synthetic-secret";
    const std::string blobUrlString = "https://account.blob.core.windows.net/container/blob";

    Blobs::BlobClientOptions clientOptions
        = InitSyntheticClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
    auto options = InitSyntheticClientOptions<Azure::Identity::ClientSecretCredentialOptions>();

    // With tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    auto blobClient = Blobs::BlobClient(
        blobUrlString,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            aadTenantId, aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());
    EXPECT_NO_THROW(ReadBodyStream(blobClient.Download().Value.BodyStream));

    // Without tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    blobClient = Blobs::BlobClient(
        blobUrlString,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());

    // With custom audience
    auto blobUrl = Azure::Core::Url(blobUrlString);
    clientOptions.Audience = Blobs::BlobAudience(blobUrl.GetScheme() + "://" + blobUrl.GetHost());
    blobClient = Blobs::BlobClient(
        blobUrlString,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());
    clientOptions.Audience.Reset();

    // With wrong tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    blobClient = Blobs::BlobClient(
        blobUrlString,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "test", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());

    // Disable Tenant Discovery and without tenantId
    clientOptions.EnableTenantDiscovery = false;
    blobClient = Blobs::BlobClient(
        blobUrlString,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_THROW(blobClient.GetProperties(), Azure::Core::Credentials::AuthenticationException);

    // Don't allow additional tenants
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {};
    blobClient = Blobs::BlobClient(
        blobUrlString,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_THROW(blobClient.GetProperties(), Azure::Core::Credentials::AuthenticationException);
    ExpectTokenScopes("https://account.blob.core.windows.net/.default");
  }

}}} // namespace Azure::Storage::Test
