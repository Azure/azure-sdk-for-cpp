// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/identity/environment_credential.hpp>
#include <azure/service/client.hpp>

#include <iostream>

int main()
{
  try
  {
    // To diagnose, see https://aka.ms/azsdk/cpp/identity/troubleshooting
    // For example, try setting 'AZURE_LOG_LEVEL' environment variable to 'verbose' before running
    // this sample to see more details.

    // Step 1: Create an EnvironmentCredential instance.
    // Environment Credential would read its parameters from the environment variables, such as
    // AZURE_TENANT_ID, AZURE_CLIENT_ID, AZURE_CLIENT_SECRET. See documentation for details.
    auto environmentCredential = std::make_shared<Azure::Identity::EnvironmentCredential>();

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", environmentCredential);

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
