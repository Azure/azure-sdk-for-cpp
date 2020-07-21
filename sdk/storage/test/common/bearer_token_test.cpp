// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob.hpp"
#include "credentials/credentials.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(ClientSecretCredentialTest, ClientSecretCredentialWorks)
  {
    const std::string containerName = "bearertokentest";

    if (ClientId().empty() || ClientSecret().empty() || TenantId().empty())
    {
      return;
    }

    auto credential = std::make_shared<Azure::Core::Credentials::ClientSecretCredential>(
        TenantId(), ClientId(), ClientSecret());

    auto containerClient = Azure::Storage::Blobs::BlobContainerClient(
        Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
            StandardStorageConnectionString(), containerName)
            .GetUri(),
        credential);

    EXPECT_NO_THROW(containerClient.Create());
    EXPECT_NO_THROW(containerClient.Delete());
  }

}}} // namespace Azure::Storage::Test