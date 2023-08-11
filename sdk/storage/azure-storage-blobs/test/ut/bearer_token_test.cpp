// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlockBlobClientTest, ClientSecretCredentialWorks)
  {
    const std::string containerName = LowercaseRandomString();
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName);
    auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        AadTenantId(),
        AadClientId(),
        AadClientSecret(),
        InitStorageClientOptions<Core::Credentials::TokenCredentialOptions>());
    containerClient = Blobs::BlobContainerClient(
        containerClient.GetUrl(), credential, InitStorageClientOptions<Blobs::BlobClientOptions>());

    EXPECT_NO_THROW(containerClient.Create());
    EXPECT_NO_THROW(containerClient.Delete());
  }

  TEST_F(BlockBlobClientTest, BearerChallengeWorks)
  {
    Blobs::BlobClientOptions clientOptions
        = InitStorageClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
    Azure::Identity::ClientSecretCredentialOptions options;

    // With tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    auto blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret(), options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());
    EXPECT_NO_THROW(ReadBodyStream(blobClient.Download().Value.BodyStream));

    // Without tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", AadClientId(), AadClientSecret(), options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());

    // With error tenantId
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {"*"};
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "test", AadClientId(), AadClientSecret(), options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());

    // Disable Tenant Discovery and without tenantId
    clientOptions.EnableTenantDiscovery = false;
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", AadClientId(), AadClientSecret(), options),
        clientOptions);
    EXPECT_THROW(blobClient.GetProperties(), Azure::Core::Credentials::AuthenticationException);

    // Don't allow additional tenants
    clientOptions.EnableTenantDiscovery = true;
    options.AdditionallyAllowedTenants = {};
    blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            "", AadClientId(), AadClientSecret(), options),
        clientOptions);
    EXPECT_THROW(blobClient.GetProperties(), Azure::Core::Credentials::AuthenticationException);
  }

}}} // namespace Azure::Storage::Test
