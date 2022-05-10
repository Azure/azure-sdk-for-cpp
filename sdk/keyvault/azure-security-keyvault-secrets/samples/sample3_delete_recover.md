# Delete and recover deleted secrets

This sample demonstrates how to delete and recover a deleted secret in Azure Key Vault.

## Creating a SecretClient

To create a new `SecretClient` to create, get, update, or delete secrets, you need the endpoint to an Azure Key Vault and credentials.

The simplest way of providing a bearer token is to use the `EnvironmentCredential` authentication method by providing client secret credentials is being used in this
getting started section, but you can find more ways to authenticate with [azure-identity](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity).

```cpp Snippet:SecretSample3CreateCredential
  auto credential = std::make_shared<Azure::Identity::EnvironmentCredential>();
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SecretSample3SecretClient
SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Secret

Call SetSecret to create a new secret with name and secret value.

```cpp Snippet:SecretSample3SetSecret
std::string secretName("MySampleSecret");
std::string secretValue("my secret value");

secretClient.SetSecret(secretName, secretValue);
```

## Getting a Secret

Call GetSecret to retrieve a secret from Key Vault.

```cpp Snippet:SecretSample3GetSecret
// get secret
Secret secret = secretClient.GetSecret(secretName).Value;
std::cout << "Secret is returned with name " << secret.Name << " and value " << secret.Value
          << std::endl;
```

## Deleting a secret

Call StartDeleteSecret to delete a secret. This is a long running operation.

```cpp Snippet:SecretSample3DeleteSecret
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);
```

## Recover a Deleted secret

Call StartRecoverDeletedSecret to recover a deleted secret and then poll until the operation is done.

```cpp Snippet:SecretSample3RecoverSecret
// call restore secret
RecoverDeletedSecretOperation recoverOperation = secretClient.StartRecoverDeletedSecret(secret.Name);

// poll until done
Secret restoredSecret = recoverOperation.PollUntilDone(2s).Value;
```

## Source

To see the full example source, see:
[Source Code](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/keyvault/azure-security-keyvault-secrets/test/samples/sample3-delete-recover)

