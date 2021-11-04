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
#include <thread>

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
    CertificateCreateOptions options;
    // setup certificate create properties/policy
    {
      // create a lifetime action
      LifetimeAction action;
      action.LifetimePercentage = 80;
      action.Action = CertificatePolicyAction::AutoRenew;

      // etu properties
      options.Properties.Enabled = true;
      options.Properties.Name = certificateName;

      // setup policy
      options.Policy.Subject = "CN=sample1";
      options.Policy.ValidityInMonths = 12;
      options.Policy.Enabled = true;
      options.Policy.ContentType = CertificateContentType::Pkcs12;
      options.Policy.IssuerName = "Self";

      // add a lifetime action
      options.Policy.LifetimeActions.emplace_back(action);
    }
    // create a certificate
    {
      // start the create process
      auto response = certificateClient.StartCreateCertificate(certificateName, options);
      // wait for complete to get the certificate
      certificate = response.PollUntilDone(defaultWait).Value;

      std::cout << "Created certificate with policy. Certificate name : " << certificate.Name();
    }
    // update certificate
    {
      std::cout << "Certificate is enabled : "
                << (certificate.Properties.Enabled.Value() ? "true" : "false");
      CertificateProperties updateOptions;
      updateOptions = certificate.Properties;
      updateOptions.Enabled = false;

      auto updatedCertificate
          = certificateClient
                .UpdateCertificateProperties(
                    certificateName, certificate.Properties.Version, updateOptions)
                .Value;

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
