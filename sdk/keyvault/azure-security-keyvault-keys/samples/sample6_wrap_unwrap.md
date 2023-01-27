# _Creating, wrapping and unwrapping keys

This sample demonstrates how to create, get, wrap and unwrap a key in Azure Key Vault.
To get started, you'll need a URI to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/README.md) for links and instructions.

## _Creating a KeyClient

To create a new `KeyClient` to create, get, update, or delete keys, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Keys client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and client secret as environment variables.

```cpp Snippet:KeysSample6CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:KeysSample6KeyClient
KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## _Creating a key

Let's create an RSA key valid for 1 year.
If the key already exists in the Azure Key Vault, then a new version of the key is created.

```cpp Snippet:KeysSample6CreateKey
auto rsaKey = CreateRsaKeyOptions(rsaKeyName);
rsaKey.KeySize = 2048;
rsaKey.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(24 * 365);

keyClient.CreateRsaKey(rsaKey);
```

## _Creating the CryptographyClient

Let's create a CryptographyClient instance using the created key.

```cpp Snippet:KeysSample6CryptoClient
CryptographyClient cryptoClient(cloudRsaKey.Id(), credential);

```

## _Wrap the key

Now we will wrap the key.
```cpp Snippet:KeysSample6WrapKey
// keyDataSource simulates a symmetric private key created locally in the system. It is not
// relevant for the sample how to create the private key as it depends on the OS.
// For example, on linux, the key can be created using openSSL.
uint8_t const keyDataSource[]
    = "MIIBOgIBAAJBAKUFtjMCrEZzg30Rb5EQnFy6fFUTn3wwVPM9yW4Icn7EMk34ic+"
      "3CYytbOqbRQDDUtbyUCdMEu2OZ0RPqL4GWMECAwEAAQJAcHi7HHs25XF3bbeDfbB/"
      "kae8c9PDAEaEr6At+......";
std::vector<uint8_t> keyData(std::begin(keyDataSource), std::end(keyDataSource));
std::cout << " - Using a sample generated key: " << Azure::Core::Convert::Base64Encode(keyData)
          << std::endl;

auto wrapResult = cryptoClient.WrapKey(KeyWrapAlgorithm::RsaOaep, keyData).Value;
std::cout << " - Encrypted data using the algorithm " << wrapResult.Algorithm.ToString()
          << ", with key " << wrapResult.KeyId << ". The resulting encrypted data is: "
          << Azure::Core::Convert::Base64Encode(wrapResult.EncryptedKey) << std::endl;

```

## _Unwrap the key

Let's unwrap the key.
```cpp Snippet:KeysSample6UnwrapKey
auto unwrapResult
    = cryptoClient.UnwrapKey(KeyWrapAlgorithm::RsaOaep, wrapResult.EncryptedKey).Value;
std::cout << " - Decrypted data using the algorithm " << unwrapResult.Algorithm.ToString()
          << ", with key " << unwrapResult.KeyId << ". The resulting decrypted data is: "
          << Azure::Core::Convert::Base64Encode(unwrapResult.Key) << std::endl;
```

## _Deleting a key

The cloud RSA key is no longer needed, so we need to delete it from the Key Vault.

```cpp Snippet:KeysSample1DeleteKey
DeleteKeyOperation operation = keyClient.StartDeleteKey(rsaKeyName);
```

## _Purging a deleted key

If the Azure Key Vault is soft delete-enabled and you want to permanently delete the key before its `ScheduledPurgeDate`,
the deleted key needs to be purged. Before it can be purged, you need to wait until the key is fully deleted.

```cpp Snippet:KeysSample1PurgeKey
// You only need to wait for completion if you want to purge or recover the key.
operation.PollUntilDone(std::chrono::milliseconds(2000));

keyClient.PurgeDeletedKey(rsaKeyName);
```

## _Source

- [sample6_wrap_unwrap.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/test/samples/sample6-wrap-unwrap/sample6_wrap_unwrap.cpp)

[defaultazurecredential]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/identity/azure-identity/README.md
