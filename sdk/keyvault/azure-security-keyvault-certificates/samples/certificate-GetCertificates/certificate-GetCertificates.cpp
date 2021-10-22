// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get properties of certificates, get properties of certificate versions, delete
 * , get deleted certificates, purge
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

KeyVaultCertificateWithPolicy CreateCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient)
{
  CertificateCreateParameters params;
  std::chrono::milliseconds defaultWait(10s);
  // setup certificate create properties/policy
  {
    // create a lifetime action
    LifetimeAction action;
    action.LifetimePercentage = 80;
    action.Action = CertificatePolicyAction::AutoRenew;

    // setup properties
    params.Properties.Enabled = true;
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
    params.Properties.Name = certificateName;
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
    auto certificate = certificateClient.GetCertificate(certificateName).Value;

    std::cout << "Created certificate with policy. Certificate name : " << certificate.Name();

    return certificate;
  }
}

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
    std::string certificateName1 = "Sample1";
    std::string certificateName2 = "Sample2";
    KeyVaultCertificateWithPolicy certificate1;
    KeyVaultCertificateWithPolicy certificate2;
    // create and get two certificates
    {
      // create certificates
      certificate1 = CreateCertificate(certificateName1, certificateClient);
      certificate2 = CreateCertificate(certificateName2, certificateClient);

      // get certificate properties
      auto certificates
          = certificateClient.GetPropertiesOfCertificates(GetPropertiesOfCertificatesOptions());

      std::cout << "Found " << certificates.Items.size() << " certificates.";
      for (auto oneCertificate : certificates.Items)
      {
        std::cout << "Certificate name : " << oneCertificate.Name;
      }
    }
    // certificate versions, and get versions
    {
      // create new version of certificate
      CreateCertificate(certificateName1, certificateClient);

      // get properties of all the versions of a certificate
      auto certificateVersions
          = certificateClient.GetPropertiesOfCertificateVersions(certificateName1);

      std::cout << "Found " << certificateVersions.Items.size()
                << " certificate versions for certificate " << certificateName1;
    }
    // delete the certificates, and get deleted
    {
      // delete the certificates
      auto response1 = certificateClient.StartDeleteCertificate(certificateName1);
      auto response2 = certificateClient.StartDeleteCertificate(certificateName2);
      response1.PollUntilDone(defaultWait);
      response2.PollUntilDone(defaultWait);

      // get properties of deleted certificates
      auto deletedCertificates = certificateClient.GetDeletedCertificates();

      std::cout << "Found " << deletedCertificates.Items.size() << " deleted certificates.";
    }
    // purge the certificates
    {
      certificateClient.PurgeDeletedCertificate(certificateName1);
      certificateClient.PurgeDeletedCertificate(certificateName2);
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
