// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <azure/identity.hpp>
#include <azure/keyvault/keyvault_certificates.hpp>

#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Certificates;
using namespace std::chrono_literals;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
  std::chrono::milliseconds defaultWait(10s);
  // create client
  CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  try
  {
    std::string certificateName = "Sample1";
    KeyVaultCertificateWithPolicy certificate;
    auto params = CertificateCreateParameters();
    // setup certificate create properties/policy
    {

      // create a lifetime action
      LifetimeAction action;
      action.LifetimePercentage = 80;
      action.Action = CertificatePolicyAction::AutoRenew;

      // etu properties
      params.Properties.Enabled = true;
      params.Properties.Name = certificateName;

      // setup policy
      params.Policy.Subject = "CN=sample1";
      params.Policy.ValidityInMonths = 12;
      params.Policy.Enabled = true;
      params.Policy.ContentType = CertificateContentType::Pkcs12;
      params.Policy.IssuerName = "Self";

      // add a lifetime action
      params.Policy.LifetimeActions.emplace_back(action);
    }
    // create a certificate
    {
      // start the create process
      auto response = certificateClient.StartCreateCertificate(certificateName, params);
      auto result = response.PollUntilDone(defaultWait);

      // check that the operation completed
      while (!response.IsCompleted())
      {
        response.UpdateProperties();
        std::this_thread::sleep_for(defaultWait);
      }

      // get the certificate
      certificate = certificateClient.GetCertificate(certificateName).Value;

      std::cout << "Created certificate with policy. Certificate name : " << certificate.Name();
    }
    // update certificate
    {
      std::cout << "Certificate is enabled : "
                << (certificate.Properties.Enabled.Value() ? "true" : "false");
      CertificateUpdateOptions updateOptions;
      updateOptions.Properties = certificate.Properties;
      updateOptions.Properties.Enabled = false;

      auto updatedCertificate = certificateClient.UpdateCertificateProperties(updateOptions).Value;

      std::cout << "After update certificate is enabled : "
                << (updatedCertificate.Properties.Enabled.Value() ? "true" : "false");
    }
    // delete the certificate
    {
      auto response = certificateClient.StartDeleteCertificate(certificateName);
      auto result = response.PollUntilDone(defaultWait);
      certificateClient.PurgeDeletedCertificate(certificateName);
    }
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Key Vault Certificate Client Exception happened:" << std::endl
              << e.Message << std::endl;
    return 1;
  }
  return 0;
}
