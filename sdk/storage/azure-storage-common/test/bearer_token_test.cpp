// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/blobs.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(ClientSecretCredentialTest, ClientSecretCredentialWorks)
  {
    const std::string containerName = "bearertokentest" + LowercaseRandomString();

    EXPECT_FALSE(AadClientId().empty() || AadClientSecret().empty() || AadTenantId().empty());

    auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        AadTenantId(), AadClientId(), AadClientSecret());

    auto containerClient = Azure::Storage::Blobs::BlobContainerClient(
        Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
            StandardStorageConnectionString(), containerName)
            .GetUri(),
        credential);

    EXPECT_NO_THROW(containerClient.Create());
    EXPECT_NO_THROW(containerClient.Delete());
  }

}}} // namespace Azure::Storage::Test