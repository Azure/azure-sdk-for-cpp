// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 *
 */

#include <azure/identity.hpp>
#include <azure/keyvault/certificates.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::KeyVault::Certificates;
using namespace std::chrono_literals;

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  std::chrono::milliseconds defaultWait(10s);
  // create client
  CertificateClient certificateClient(keyVaultUrl, credential);

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
      auto pollResponse = response.PollUntilDone(defaultWait).Value;
      // check the status of the poll response
      if (!pollResponse.Error && pollResponse.Status.Value() == "completed")
      {
        // get the certificate
        certificate = certificateClient.GetCertificate(certificateName).Value;
        std::cout << "Created certificate with policy. Certificate name : " << certificate.Name();
      }
      else
      {
        std::cout << "Create certificate with policy result : " << pollResponse.Status.Value();
      }
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
                << (updatedCertificate.Properties.Enabled.HasValue()
                            && updatedCertificate.Properties.Enabled.Value()
                        ? "true"
                        : "false");
    }
    // delete the certificate
    {
      auto response = certificateClient.StartDeleteCertificate(certificateName);
      auto result = response.PollUntilDone(defaultWait);
      // since there is a potential delay in the delete process, we need to check the status of
      // purge
      bool retry = true;
      int retries = 5;
      while (retries > 0 && retry)
      {
        try
        {
          retries--;
          certificateClient.PurgeDeletedCertificate(certificateName);
          retry = false;
        }
        catch (Azure::Core::RequestFailedException const& e)
        {
          retry = (e.StatusCode == Azure::Core::Http::HttpStatusCode::Conflict);
          if (!retry)
          {
            throw e;
          }
          std::this_thread::sleep_for(std::chrono::seconds(15));
        }
      }
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
