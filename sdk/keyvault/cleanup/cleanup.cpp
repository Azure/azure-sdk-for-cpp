// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Secrets SDK client for
 * C++ to create, get, update, delete and purge a secret.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/identity.hpp>
#include <azure/keyvault/certificates.hpp>
#include <azure/keyvault/keys.hpp>
#include <azure/keyvault/secrets.hpp>
#include <chrono>
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Security::KeyVault::Keys;
using namespace std::chrono_literals;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  // create client
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  CertificateClient certClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  try
  {
    std::vector<DeleteCertificateOperation> certOps;
    std::vector<DeleteKeyOperation> keyOps;
    std::vector<DeleteSecretOperation> secretOps;
    for (auto keys = keyClient.GetPropertiesOfKeys(); keys.HasPage(); keys.MoveToNextPage())
    {
      for (auto const& key : keys.Items)
      {
        try
        {
          keyOps.push_back(keyClient.StartDeleteKey(key.Name));
          std::cout << "DeleteKey" << key.Name << std::endl;
        }
        catch (...)
        {
          std::cout << "fail to delete key " << key.Name;
        }
      }
    }

    for (auto secrets = secretClient.GetPropertiesOfSecrets(); secrets.HasPage();
         secrets.MoveToNextPage())
    {
      for (auto const& secret : secrets.Items)
      {
        try
        {
          secretOps.push_back(secretClient.StartDeleteSecret(secret.Name));
          std::cout << "DeleteSecret" << secret.Name << std::endl;
        }
        catch (...)
        {
          std::cout << "fail to delete secret " << secret.Name;
        }
      }
    }

    for (auto certificates = certClient.GetPropertiesOfCertificates(); certificates.HasPage();
         certificates.MoveToNextPage())
    {
      for (auto const& certificate : certificates.Items)
      {
        try
        {
          certOps.push_back(certClient.StartDeleteCertificate(certificate.Name));
          std::cout << "Delete Certificate" << certificate.Name << std::endl;
        }
        catch (...)
        {
          std::cout << "fail to delete cert " << certificate.Name;
        }
      }
    }

    for (auto op : keyOps)
    {
      op.PollUntilDone(1s);
      keyClient.PurgeDeletedKey(op.Value().Name());
      std::cout << "Purge Key " << op.Value().Name() << std::endl;
    }

    for (auto op : certOps)
    {
      op.PollUntilDone(1s);
      certClient.PurgeDeletedCertificate(op.Value().Name());
      std::cout << "Purge cert " << op.Value().Name() << std::endl;
    }

    for (auto op : secretOps)
    {
      op.PollUntilDone(1s);
      secretClient.PurgeDeletedSecret(op.Value().Name);
      std::cout << "Purge secret " << op.Value().Name << std::endl;
    }
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Key Vault Secret Client Exception happened:" << std::endl
              << e.Message << std::endl;
    return 1;
  }

  return 0;
}
