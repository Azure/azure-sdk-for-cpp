// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(StorageTest, ClientSecretCredentialWorks_LIVEONLY_)
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

}}} // namespace Azure::Storage::Test