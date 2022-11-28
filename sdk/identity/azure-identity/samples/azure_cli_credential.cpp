// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/identity/azure_cli_credential.hpp>

#include <azure/service/client.hpp>

int main()
{
  try
  {
    // Step 1: Initialize Azure CLI Credential.
    // A configuration demonstrated below would authenticate using EnvironmentCredential if it is
    // available, and if it is not available, would fall back to use ManagedIdentityCredential.
    auto chainedTokenCredential = std::make_shared<Azure::Identity::AzureCliCredential>();

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", chainedTokenCredential);

    // Step 3: Start using the Azure Service Client.
    azureServiceClient.DoSomething(Azure::Core::Context::ApplicationContext);

    std::cout << "Success!" << std::endl;
  }
  catch (const Azure::Core::Credentials::AuthenticationException& exception)
  {
    // Step 4: Handle authentication errors, if needed
    // (Azure CLI invocation errors or process timeout).
    std::cout << "Authentication error: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
