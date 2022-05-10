# Creating, getting, updating, and deleting secrets

This sample demonstrates how to create, get, update, and delete and purge a secret in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault.

## Creating a SecretClient

To create a new `SecretClient` to create, get, update, or delete secrets, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Secrets client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:SecretSample1CreateCredential
  auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SecretSample1SecretClient
SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Secret

Call SetSecret to create a new secret with name and secret value.

```cpp Snippet:SecretSample1SetSecret
std::string secretName("MySampleSecret");
std::string secretValue("my secret value");

secretClient.SetSecret(secretName, secretValue);
```

## Getting a Secret

Call GetSecret to retrieve a secret from Key Vault.

```cpp Snippet:SecretSample1GetSecret
// get secret
Secret secret = secretClient.GetSecret(secretName).Value;
std::cout << "Secret is returned with name " << secret.Name << " and value " << secret.Value
          << std::endl;
```

## Updating secret properties

Call UpdateSecretProperties to change on of the secret properties.


```cpp Snippet:SecretSample1UpdateSecretProperties
// change one of the properties
secret.Properties.ContentType = "my content";
// update the secret
Secret updatedSecret = secretClient.UpdateSecretProperties(secret.Name, secret.Properties.Version, secret.Properties)
          .Value;
std::cout << "Secret's content type is now " << updatedSecret.Properties.ContentType.Value()
          << std::endl;
```

## Deleting a secret

Call StartDeleteSecret to delete a secret. This is a long running operation.

```cpp Snippet:SecretSample1DeleteSecret
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);
```

## Purging a deleted secret

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the secret before its `ScheduledPurgeDate`, the secret needs to be purged.

```cpp Snippet:SecretSample1PurgeSecret
// You only need to wait for completion if you want to purge or recover the secret.
operation.PollUntilDone(2s);

// purge the deleted secret
secretClient.PurgeDeletedSecret(secret.Name);
```

## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-secrets/test/samples/sample1-basic-operations)
