// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlockBlobClientTest, ClientSecretCredentialWorks_LIVEONLY_)
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

  TEST_F(BlockBlobClientTest, BearerChallengeWorks_LIVEONLY_)
  {
    Blobs::BlobClientOptions clientOptions;
    clientOptions.EnableTenantDiscovery = true;
    Azure::Identity::ClientSecretCredentialOptions options;
    options.AdditionallyAllowedTenants = {"*"};
    auto blobClient = Blobs::BlobClient(
        m_blockBlobClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret(), options),
        clientOptions);
    EXPECT_NO_THROW(blobClient.GetProperties());
    EXPECT_NO_THROW(ReadBodyStream(blobClient.Download().Value.BodyStream));
  }

}}} // namespace Azure::Storage::Test