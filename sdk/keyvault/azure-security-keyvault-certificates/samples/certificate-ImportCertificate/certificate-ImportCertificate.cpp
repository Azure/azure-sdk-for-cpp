// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to import a certificate.
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

#include "certificate-ImportCertificate.hpp"
#include <azure/core/http/http_status_code.hpp>
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
    // certificate names
    std::string pemName = "Pem1";
    std::string pkcsName = "Pkcs1";
    // import pem certificate
    {
      // prepare the parameters
      auto params = ImportCertificateOptions();
      params.Value = pemCertificate;

      params.Policy.Enabled = true;
      params.Policy.KeyType = CertificateKeyType::Rsa;
      params.Policy.KeySize = 2048;
      params.Policy.ContentType = CertificateContentType::Pem;
      params.Policy.Exportable = true;
      // call import API
      auto imported = certificateClient.ImportCertificate(pemName, params).Value;
      // get some value from the certificate
      std::cout << "Imported pem certificate with name " << imported.Name();
    }
    // import pkcs certificate
    {
      // prepare the parameters
      auto params = ImportCertificateOptions();
      params.Value = pkcsBase64;

      params.Policy.Enabled = true;
      params.Policy.KeyType = CertificateKeyType::Rsa;
      params.Policy.KeySize = 2048;
      params.Policy.ContentType = CertificateContentType::Pkcs12;
      params.Policy.Exportable = true;
      // call the import API
      auto imported = certificateClient.ImportCertificate(pkcsName, params).Value;
      // read something from the certificate
      std::cout << "Imported pkcs certificate with name " << imported.Name();
    }
    // delete the certificates, and get deleted
    {
      // delete the certificates
      auto response1 = certificateClient.StartDeleteCertificate(pemName);
      auto response2 = certificateClient.StartDeleteCertificate(pkcsName);
      response1.PollUntilDone(defaultWait);
      response2.PollUntilDone(defaultWait);
      // purge the certificates
      certificateClient.PurgeDeletedCertificate(pkcsName);
      certificateClient.PurgeDeletedCertificate(pemName);
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
