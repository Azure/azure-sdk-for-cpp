# Delete and recover deleted secrets

This sample demonstrates how to delete and recover a deleted secret in Azure Key Vault.

## Creating a SecretClient

To create a new `SecretClient` to create, get, update, or delete secrets, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Secrets client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:SecretSample3CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SecretSample3SecretClient
SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Secret

To create a secret all you need to set id the name and secret value.

```cpp Snippet:SecretSample3SetSecret
std::string secretName("MySampleSecret");
std::string secretValue("my secret value");

secretClient.SetSecret(secretName, secretValue);
```

## Getting a Secret

To get a secret from the keyvault  you will need to call GetSecret.

```cpp Snippet:SecretSample3GetSecret
// get secret
Secret secret = secretClient.GetSecret(secretName).Value;
std::cout << "Secret is returned with name " << secret.Name << " and value " << secret.Value
          << std::endl;
```

## Deleting a secret

The secret is no longer needed so we need to delete it.

```cpp Snippet:SecretSample3DeleteSecret
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);
```

## Recover a Deleted secret

To recover a deleted secret we need to call StartRecoverDeletedSecret and then poll untill the operation is done.

```cpp Snippet:SecretSample3RecoverSecret
// call restore secret
RecoverDeletedSecretOperation recoverOperation = secretClient.StartRecoverDeletedSecret(secret.Name);

// poll until done
Secret restoredSecret = recoverOperation.PollUntilDone(2s).Value;
```

## Source

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md
