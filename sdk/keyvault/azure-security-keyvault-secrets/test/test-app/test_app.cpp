// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "azure/keyvault/keyvault_secrets.hpp"
#include <azure/identity.hpp>

using namespace Azure::Security::KeyVault::Secrets;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  // just a response, with a secret
  // auto response = secretClient.GetSecret("testSecret");
  // response.Value.Properties.ContentType = "content";
  // GetSecretOptions options;

  // response = secretClient.UpdateSecretProperties(
  //     response.Value.Name, response.Value.Properties.Version, response.Value.Properties);

  // just a response, with a secret
  // auto response3 = secretClient.GetDeletedSecret("someSecret");

  // auto response4 = secretClient.BackupSecret("someSecret2");
  // auto response5 = secretClient.RestoreSecretBackup(response4.Value.Secret);

  // auto response = secretClient.PurgeDeletedSecret("someSecret3");

  // auto response4 = secretClient.BackupSecret("someSecret2");
  // auto response5 = secretClient.RestoreSecretBackup(response4.Value.Secret);

  auto response6 = secretClient.StartRecoverDeletedSecret("someSecret2");
  if (!response6.IsDone())
  {
    auto resumeToken = response6.GetResumeToken();
    auto response7 = response6.CreateFromResumeToken(resumeToken, secretClient);
    auto response8 = response7.Poll();
  }
  // auto response4 = secretClient.BackupSecret("someSecret2");
  // auto response5 = secretClient.RestoreSecretBackup(response4.Value.Secret);
  // GetPropertiesOfSecretsOptions options;
  // options.MaxResults = 1;
  // auto r1 = secretClient.GetPropertiesOfSecrets(options);
  // auto r2 = secretClient.GetPropertiesOfSecretsVersions(r1.Items[0].Name);
  auto r3 = secretClient.GetDeletedSecrets();
  // r1.MoveToNextPage();
  return 0;
}
