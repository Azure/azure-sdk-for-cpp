// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/identity/azure_cli_credential.hpp>
#include <azure/identity/chained_token_credential.hpp>
#include <azure/identity/environment_credential.hpp>
#include <azure/identity/managed_identity_credential.hpp>
#include <azure/service/client.hpp>

#include <iostream>

int main()
{
  try
  {
    // Step 1: Initialize Chained Token Credential.
    // A configuration demonstrated below would authenticate using EnvironmentCredential if it is
    // available, and if it is not available, would fall back to use AzureCliCredential, and then to
    // ManagedIdentityCredential.
    auto chainedTokenCredential = std::make_shared<Azure::Identity::ChainedTokenCredential>(
        Azure::Identity::ChainedTokenCredential::Sources{
            std::make_shared<Azure::Identity::EnvironmentCredential>(),
            std::make_shared<Azure::Identity::AzureCliCredential>(),
            std::make_shared<Azure::Identity::ManagedIdentityCredential>()});

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", chainedTokenCredential);

    // Step 3: Start using the Azure Service Client.
    azureServiceClient.DoSomething(Azure::Core::Context::ApplicationContext);

    std::cout << "Success!" << std::endl;
  }
  catch (const Azure::Core::Credentials::AuthenticationException& exception)
  {
    // Step 4: Handle authentication errors, if needed
    // (invalid credential parameters, insufficient permissions).
    std::cout << "Authentication error: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
