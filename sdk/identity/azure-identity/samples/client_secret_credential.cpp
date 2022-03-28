// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/identity/client_secret_credential.hpp>

#include <azure/service/client.hpp>

// These functions should be getting the real Tenant ID, Client ID, and the Client Secret to
// authenticate. It is recommended to NOT hardcode the secret in the code, but to get it from the
// environment or read it from a secure location.
std::string GetTenantId() { return std::string(); }
std::string GetClientId() { return std::string(); }
std::string GetClientSecret() { return std::string(); }

int main()
{
  try
  {
    // Step 1: Initialize Client Secret Credential.
    auto clientSecretCredential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        GetTenantId(), GetClientId(), GetClientSecret());

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", clientSecretCredential);

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
