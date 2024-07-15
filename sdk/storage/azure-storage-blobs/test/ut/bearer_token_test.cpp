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

  TEST_F(BlockBlobClientTest, DISABLED_BearerChallengeWorks)
  {
    // This testcase needs a client secret to run.
    const std::string aadTenantId = "";
    const std::string aadClientId = "";
    const std::string aadClientSecret = "";

    Blobs::BlobClientOptions clientOptions
        = InitStorageClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
    auto options = Azure::Identity::ClientSecretCredentialOptions();

    // With tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    auto blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            aadTenantId, aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());
    EXPECT_NO_THROW(ReadBodyStream(blobClient.Download().Value.BodyStream));

    // Without tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());

    // With custom audience
    auto blobUrl = Azure::Core::Url(m_blockBlobClient->GetUrl());
    clientOptions.Audience = Blobs::BlobAudience(blobUrl.GetScheme() + "://" + blobUrl.GetHost());
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());
    clientOptions.Audience.Reset();

    // With wrong tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "test", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());

    // Disable Tenant Discovery and without tenantId
    clientOptions.EnableTenantDiscovery = false;
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_THROW(blobClient.GetProperties(), Azure::Core::Credentials::AuthenticationException);

    // Don't allow additional tenants
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {};
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", aadClientId, aadClientSecret, options),
        clientOptions);
    EXPECT_THROW(blobClient.GetProperties(), Azure::Core::Credentials::AuthenticationException);
  }

}}} // namespace Azure::Storage::Test
