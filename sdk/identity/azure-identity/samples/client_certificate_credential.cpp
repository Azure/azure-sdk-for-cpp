// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/identity/client_certificate_credential.hpp>

#include <azure/service/client.hpp>

// These functions should be getting the real Tenant ID, Client ID, and the Client Certificate to
// authenticate.
std::string GetTenantId() { return std::string(); }
std::string GetClientId() { return std::string(); }
std::string GetClientCertificatePath() { return std::string(); }

int main()
{
  try
  {
    // Step 1: Initialize Client Certificate Credential.
    auto clientCertificateCredential
        = std::make_shared<Azure::Identity::ClientCertificateCredential>(
            GetTenantId(), GetClientId(), GetClientCertificatePath());

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", clientCertificateCredential);

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
