// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/identity/managed_identity_credential.hpp>
#include <azure/service/client.hpp>
#include <azure/storage/blobs.hpp>

#include <iostream>

static void ShowDifferentManagedIdentityApproaches()
{
  using namespace Azure::Identity;
  using namespace Azure::Storage::Blobs;

  std::string blobUrl = "https://myaccount.blob.core.windows.net/mycontainer/myblob";
  {
    // @begin_snippet: UserAssignedManagedIdentityViaClientId
    // When deployed to an Azure host, ManagedIdentityCredential will authenticate the specified
    // user-assigned managed identity.

    std::string userAssignedClientId = "<your managed identity client ID>";
    ManagedIdentityCredentialOptions options;
    options.IdentityId = ManagedIdentityId(ManagedIdentityIdKind::ClientId, userAssignedClientId);

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: UserAssignedManagedIdentityViaResourceId
    std::string userAssignedResourceId = "<your managed identity resource ID>";
    ManagedIdentityCredentialOptions options;
    options.IdentityId
        = ManagedIdentityId(ManagedIdentityIdKind::ResourceId, userAssignedResourceId);

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: UserAssignedManagedIdentityViaObjectId
    std::string userAssignedObjectId = "<your managed identity object ID>";
    ManagedIdentityCredentialOptions options;
    options.IdentityId = ManagedIdentityId(ManagedIdentityIdKind::ObjectId, userAssignedObjectId);

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: SystemAssignedManagedIdentity
    ManagedIdentityCredentialOptions options;
    options.IdentityId = ManagedIdentityId(ManagedIdentityIdKind::SystemAssigned, {});

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: SystemAssignedManagedIdentityBrief
    auto credential = std::make_shared<ManagedIdentityCredential>();
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
}

int main()
{
  try
  {
    // Step 1: Create a ManagedIdentityCredential instance.
    // Managed Identity Credential would be available in some environments such as on Azure VMs.
    // See documentation for details.
    auto managedIdentityCredential = std::make_shared<Azure::Identity::ManagedIdentityCredential>();

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", managedIdentityCredential);

    // Step 3: Start using the Azure Service Client.
    azureServiceClient.DoSomething();

    std::cout << "Success!" << std::endl;
  }
  catch (const Azure::Core::Credentials::AuthenticationException& exception)
  {
    // Step 4: Handle authentication errors, if needed
    // (invalid credential parameters, insufficient permissions).
    std::cout << "Authentication error: " << exception.what() << std::endl;
    return 1;
  }

  ShowDifferentManagedIdentityApproaches();

  return 0;
}
