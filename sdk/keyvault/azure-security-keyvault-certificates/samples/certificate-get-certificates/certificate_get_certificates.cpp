// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 *
 * @brief This sample provides examples of handling paged operations.
 * @details This sample provides the code implementation to use the Key Vault Certificates SDK
 * client for C++ to create, get properties of certificates, get properties of certificate versions,
 * delete , get deleted certificates, purge
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

KeyVaultCertificateWithPolicy CreateCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient);

void PurgeCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient);

int main()
{
  auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
  auto credential = std::make_shared<Azure::Identity::AzureCliCredential>();
  std::chrono::milliseconds defaultWait(10s);
  // create client
  CertificateClient certificateClient(keyVaultUrl, credential);

  try
  {
    std::string certificateName1 = "SampleCertificate1";
    std::string certificateName2 = "SampleCertificate2";
    KeyVaultCertificateWithPolicy certificate1;
    KeyVaultCertificateWithPolicy certificate2;
    // create and get two certificates
    {
      // create certificates
      certificate1 = CreateCertificate(certificateName1, certificateClient);
      certificate2 = CreateCertificate(certificateName2, certificateClient);

      // get properties of certificates
      for (auto certificates = certificateClient.GetPropertiesOfCertificates();
           certificates.HasPage();
           certificates.MoveToNextPage())
      { // go through every certificate of each page returned
        // the number of results returned for in a  page is not guaranteed
        // it can be anywhere from 0 to 25
        std::cout << "Found " << certificates.Items.size() << " certificates.";
        for (auto oneCertificate : certificates.Items)
        {
          std::cout << "Certificate name : " << oneCertificate.Name;
        }
      }
    }
    // certificate versions, and get versions
    {
      // create new version of certificate
      CreateCertificate(certificateName1, certificateClient);

      // get properties of all the versions of a certificate
      for (auto certificateVersions
           = certificateClient.GetPropertiesOfCertificateVersions(certificateName1);
           certificateVersions.HasPage();
           certificateVersions.MoveToNextPage())
      { // go through every certificate of each page returned
        // the number of results returned for in a  page is not guaranteed
        // it can be anywhere from 0 to 25

        std::cout << "Found " << certificateVersions.Items.size()
                  << " certificate versions for certificate " << certificateName1;
      }
    }
    // delete the certificates, and get deleted
    {
      // delete the certificates
      auto response1 = certificateClient.StartDeleteCertificate(certificateName1);
      auto response2 = certificateClient.StartDeleteCertificate(certificateName2);
      response1.PollUntilDone(defaultWait);
      response2.PollUntilDone(defaultWait);

      // get properties of deleted certificates
      for (auto deletedCertificates = certificateClient.GetDeletedCertificates();
           deletedCertificates.HasPage();
           deletedCertificates.MoveToNextPage())
      { // go through every certificate of each page returned
        // the number of results returned for in a  page is not guaranteed
        // it can be anywhere from 0 to 25
        std::cout << "Found " << deletedCertificates.Items.size() << " deleted certificates.";
      }
    }
    // purge the certificates
    {
      PurgeCertificate(certificateName1, certificateClient);
      PurgeCertificate(certificateName2, certificateClient);
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

KeyVaultCertificateWithPolicy CreateCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient)
{
  CertificateCreateOptions options;
  std::chrono::milliseconds defaultWait(10s);
  // setup certificate create properties/policy
  {
    // create a lifetime action
    LifetimeAction action;
    action.LifetimePercentage = 80;
    action.Action = CertificatePolicyAction::AutoRenew;

    // setup properties
    options.Properties.Enabled = true;
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
    options.Properties.Name = certificateName;
    // start the create process
    auto response = certificateClient.StartCreateCertificate(certificateName, options);
    // wait for complete to get the certificate
    auto pollResponse = response.PollUntilDone(defaultWait).Value;

    // check the status of the poll response
    if (!pollResponse.Error && pollResponse.Status.HasValue()
        && pollResponse.Status.Value() == "completed")
    {
      // get the certificate
      auto certificate = certificateClient.GetCertificate(certificateName).Value;
      std::cout << "Created certificate with policy. Certificate name : " << certificate.Name();
      return certificate;
    }
    else
    {
      throw std::runtime_error(
          "Create certificate with policy result : " + pollResponse.Status.Value());
    }
  }
}

void PurgeCertificate(
    std::string const& certificateName,
    CertificateClient const& certificateClient)
{
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
        throw;
      }
      std::this_thread::sleep_for(std::chrono::seconds(15));
    }
  }
}
