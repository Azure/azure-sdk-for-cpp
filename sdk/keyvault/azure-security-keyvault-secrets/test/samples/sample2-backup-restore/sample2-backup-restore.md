# Backup and Restore secrets

This sample demonstrates how to backup and restore in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault. 

## Creating a SecretClient

To create a new `SecretClient` to create, get, update, or delete secrets, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Secrets client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and Client Secret as environment variables.

```cpp Snippet:SecretSample2CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:SecretSample2SecretClient
SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a Secret

To create a secret all you need to set id the name and secret value.

```cpp Snippet:SecretSample2SetSecret
std::string secretName("MySampleSecret");
std::string secretValue("my secret value");

secretClient.SetSecret(secretName, secretValue);
```

## Getting a Secret

To get a secret from the keyvault  you will need to call GetSecret.

```cpp Snippet:SecretSample2GetSecret
// get secret
Secret secret = secretClient.GetSecret(secretName).Value;
std::cout << "Secret is returned with name " << secret.Name << " and value " << secret.Value
          << std::endl;
```

## Creating a Backup for the secret properties

In order to get the backup of the secret we need to call BackupSecret, which will return a vector of bytes representing the backed up content. 


```cpp Snippet:SecretSample2BackupSecret
std::cout << "\t-Backup Key" << std::endl;
std::vector<uint8_t> backupKey(secretClient.BackupSecret(secret.Name).Value.Secret);
backUpSize = backupKey.size();
```

## Deleting the secret in order to later restore it

The secret is no longer needed so we need to delete it.

```cpp Snippet:SecretSample2DeleteSecret
// start deleting the secret
DeleteSecretOperation operation = secretClient.StartDeleteSecret(secret.Name);
```

## Purging a deleted key

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the secret before its `ScheduledPurgeDate`, the secret needs to be purged.

```cpp Snippet:SecretSample2PurgeSecret
// You only need to wait for completion if you want to purge or recover the secret.
operation.PollUntilDone(2s);

// purge the deleted secret
secretClient.PurgeDeletedSecret(secret.Name);
```

## Restoring a secret 

In order to restore a secret we need to call RestoreSecretBackup api passing in the byte vector obtained at the previous(backup) step.

```cpp Snippet:SecretSample2RestoreSecret
std::cout << "\t-Restore Key" << std::endl;
auto restoredSecret = secretClient.RestoreSecretBackup(inMemoryBackup).Value;
```

## Source
[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md

