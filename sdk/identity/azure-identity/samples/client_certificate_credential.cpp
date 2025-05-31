// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/identity/client_certificate_credential.hpp>
#include <azure/service/client.hpp>

#include <iostream>

// The following environment variables must be set before running the sample.
// * AZURE_TENANT_ID: Tenant ID for the Azure account.
// * AZURE_CLIENT_ID: The Client ID to authenticate the request.
// * AZURE_CLIENT_CERTIFICATE_PATH: The path to a client certificate.
std::string GetTenantId() { return std::getenv("AZURE_TENANT_ID"); }
std::string GetClientId() { return std::getenv("AZURE_CLIENT_ID"); }
std::string GetClientCertificatePath() { return std::getenv("AZURE_CLIENT_CERTIFICATE_PATH"); }

int main()
{
  try
  {
    // To diagnose, see https://aka.ms/azsdk/cpp/identity/troubleshooting
    // For example, try setting 'AZURE_LOG_LEVEL' environment variable to 'verbose' before running
    // this sample to see more details.

    // Step 1: Initialize Client Certificate Credential.
    auto clientCertificateCredential
        = std::make_shared<Azure::Identity::ClientCertificateCredential>(
            GetTenantId(), GetClientId(), GetClientCertificatePath());

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", clientCertificateCredential);

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

  return 0;
}
