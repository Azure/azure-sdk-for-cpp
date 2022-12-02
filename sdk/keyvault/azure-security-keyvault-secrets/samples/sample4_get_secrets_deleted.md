# Get Secrets, Get Secrets Versions, Get Deleted Secrets, Get Deleted Secret

This sample demonstrates how to list all the secrets , all the versions of a secret, list all deleted secrets, and get the properties of a deleted secret.

## Creating a SecretClient

To create a new `SecretClient` to create, get, update, or delete secrets, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Secrets client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:SecretSample4CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SecretSample4SecretClient
SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a couple of Secrets

Call SetSecret to create a couple of new secret with names and secret values.

```cpp Snippet:SecretSample4SetSecret
std::string secretName("MySampleSecret");
std::string secretName2("MySampleSecret2");
std::string secretValue("my secret value");

Secret secret1 = secretClient.SetSecret(secretName, secretValue).Value;
Secret secret2 = secretClient.SetSecret(secretName2, secretValue).Value;
```

## Getting the properties of all the secrets in the key vault

Call GetPropertiesOfSecrets to get the properties of all the secrets in the key vault. The results of this call are paged to a maximum of 25 SecretProperties per page.

```cpp Snippet:SecretSample4ListAllSecrets
 // get properties of secrets
for (auto secrets = secretClient.GetPropertiesOfSecrets(); secrets.HasPage(); secrets.MoveToNextPage())
{ // go through every secret of each page returned
  for (auto const& secret : secrets.Items)
  {
    std::cout << "Found Secret with name: " << secret.Name << std::endl;
  }
}
```

## Get the versions of a Secret

Call GetPropertiesOfSecretsVersions in order to list all the versions of a secret. Responds similarly with a paged response of up to 25 versions of the secret per page.

```cpp Snippet:SecretSample4GetVersions
// get all the versions of a secret
for (auto secretsVersion = secretClient.GetPropertiesOfSecretsVersions(secret1.Name);
      secretsVersion.HasPage();
      secretsVersion.MoveToNextPage())
{ // go through each version of the secret
  for (auto const& secret : secretsVersion.Items)
  {
    std::cout << "Found Secret with name: " << secret.Name
              << " and with version: " << secret.Version << std::endl;
  }
}
```

## Delete both secrets

Call StartDeleteSecret to delete a secret. This is a long running operation. We shall not purge the secrets yet.

```cpp Snippet:SecretSample4DeleteSecrets
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret1.Name);
// You only need to wait for completion if you want to purge or recover the secret.
operation.PollUntilDone(2s);

// start deleting the secret
operation = secretClient.StartDeleteSecret(secret2.Name);
// You only need to wait for completion if you want to purge or recover the secret.
operation.PollUntilDone(2s);
```

## Get Deleted Secrets

Call GetDeletedSecrets to get a list of properties of all deleted secrets. This is a paged response with the same limit of 25 items per response.

```cpp Snippet:SecretSample4GetDeletedSecrets
// get all the versions of a secret
for (auto deletedSecrets = secretClient.GetDeletedSecrets(); deletedSecrets.HasPage();
      deletedSecrets.MoveToNextPage())
{ // go through each version of the secret
  for (auto const& deletedSecret : deletedSecrets.Items)
  {
    std::cout << "Found Secret with name: " << deletedSecret.Name << std::endl;
  }
}
```

## Get Deleted Secret

Call GetDeletedSecret to get information about a specific deleted secret.

```cpp Snippet:SecretSample4GetDeletedSecret
// get one deleted secret
auto deletedSecret = secretClient.GetDeletedSecret(secret1.Name);
std::cout << "Deleted Secret with name: " << deletedSecret.Value.Name;
```

## Purge the secrets to cleanup

Call PurgeDeletedSecret to finish cleaning up.

```cpp Snippet:SecretSample4PurgeSecrets
 // cleanup
secretClient.PurgeDeletedSecret(secret1.Name);
secretClient.PurgeDeletedSecret(secret2.Name);
```
## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-secrets/test/samples/sample4-get-secrets-deleted)