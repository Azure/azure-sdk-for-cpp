//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <get_env.hpp>

#include <iostream>

#include <azure/identity/client_secret_credential.hpp>

#include <azure/service/client.hpp>

// The following environment variables must be set before running the sample.
// * AZURE_TENANT_ID: Tenant ID for the Azure account.
// * AZURE_CLIENT_ID: The Client ID to authenticate the request.
// * AZURE_CLIENT_SECRET: The client secret.
std::string GetTenantId() { return std::getenv("AZURE_TENANT_ID"); }
std::string GetClientId() { return std::getenv("AZURE_CLIENT_ID"); }
std::string GetClientSecret() { return std::getenv("AZURE_CLIENT_SECRET"); }

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
