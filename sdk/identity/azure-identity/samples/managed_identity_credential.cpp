// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/identity/managed_identity_credential.hpp>

#include <azure/service/client.hpp>

int main()
{
  try
  {
    // Step 1: Create a ManagedIdentityCredential instance.
    // Managed Identity Credential would be available in some environments such as on Azure VMs.
    // See documentation for details.
    auto managedIdentityCredential = std::make_shared<Azure::Identity::ManagedIdentityCredential>();

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("some parameter", managedIdentityCredential);

    // Step 3: Start using the Azure Service Client.
    azureServiceClient.DoSomething(Azure::Core::Context::ApplicationContext);

    std::cout << "Success!" << std::endl;
  }
  catch (const Azure::Core::Credentials::AuthenticationException& exception)
  {
    // Step 4 (optional/oversimplified): Handle authentication errors
    // (invalid credential parameters, insufficient permissions).
    std::cout << "Authentication error: " << exception.what() << std::endl;
  }
}
