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
    options.IdentityId = ManagedIdentityId::FromUserAssignedClientId(userAssignedClientId);

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: UserAssignedManagedIdentityViaResourceId
    std::string userAssignedResourceId = "/subscriptions/<your managed identity resource ID>";
    ManagedIdentityCredentialOptions options;
    options.IdentityId = ManagedIdentityId::FromUserAssignedResourceId(
        Azure::Core::ResourceIdentifier(userAssignedResourceId));

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: UserAssignedManagedIdentityViaObjectId
    std::string userAssignedObjectId = "<your managed identity object ID>";
    ManagedIdentityCredentialOptions options;
    options.IdentityId = ManagedIdentityId::FromUserAssignedObjectId(userAssignedObjectId);

    auto credential = std::make_shared<ManagedIdentityCredential>(options);
    auto blobClient = BlobClient(blobUrl, credential);
    // @end_snippet
  }
  {
    // @begin_snippet: SystemAssignedManagedIdentity
    ManagedIdentityCredentialOptions options;
    options.IdentityId = ManagedIdentityId::SystemAssigned();

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
    // To diagnose, see https://aka.ms/azsdk/cpp/identity/troubleshooting
    // For example, try setting 'AZURE_LOG_LEVEL' environment variable to 'verbose' before running
    // this sample to see more details.

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
  catch (const Azure::Core::RequestFailedException& exception)
  {
    // Authentication exceptions are thrown as AuthenticationExceptions, client errors are thrown as
    // RequestFailedExceptions, so it is easier to differentiate whether the request has failed
    // due to input data, or due to authentication errors.
    std::cout << "Azure service request error: " << exception.what() << std::endl
              << "Status: " << static_cast<int>(exception.StatusCode) << " "
              << exception.ReasonPhrase << std::endl
              << "Error code: " << exception.ErrorCode << std::endl
              << "Request ID: " << exception.RequestId << std::endl
              << "Message: " << exception.Message << std::endl;
    return 2;
  }
  catch (const std::exception& exception)
  {
    std::cout << "Unexpected exception thrown: " << exception.what() << std::endl;
    return 3;
  }

  ShowDifferentManagedIdentityApproaches();

  return 0;
}
