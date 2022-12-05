# Encrypting and decrypt keys

This sample demonstrates how to encrypt and decrypt a single block of plain text with an RSA key.
To get started, you'll need a URL to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/README.md) for links and instructions.

## Creating a KeyClient

To create a new `KeyClient` to create, get, update, or delete keys, you need the endpoint to an Azure Key Vault and credentials.

Key Vault Keys client for C++ currently supports the `ClientSecretCredential` for authenticating.

In the sample below, you can create a credential by setting the Tenant ID, Client ID and client secret as environment variables.

```cpp Snippet:KeysSample1CreateCredential
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
```

Then, in the sample below, you can set `keyVaultUrl` based on an environment variable, configuration setting, or any way that works for your application.

```cpp Snippet:KeysSample1KeyClient
KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
```

## Creating a key

First, we create a RSA key which will be used to encrypt and decrypt.

```cpp
// Let's create a RSA key which will be used to encrypt and decrypt
auto rsaKeyName = "CloudRsaKey-" + Azure::Core::Uuid::CreateUuid().ToString();
auto keyOptions = CreateRsaKeyOptions(rsaKeyName, false);
keyOptions.KeySize = 2048;
KeyVaultKey cloudRsaKey = keyClient.CreateRsaKey(keyOptions).Value;
std::cout << " - Key is returned with name " << cloudRsaKey.Name() << " and type "
        << cloudRsaKey.GetKeyType().ToString() << std::endl;
```

## Creating a CryptographyClient

We create the `CryptographyClient` which can perform cryptographic operations with the key we just created using the same credential created above.

```cpp
CryptographyClient cryptoClient(cloudRsaKey.Id(), credential);
```

## Encrypting a key

Next, we'll encrypt some arbitrary plaintext with the key using the CryptographyClient.
Note that RSA encryption algorithms have no chaining so they can only encrypt a single block of plaintext securely.

```cpp
uint8_t const data[] = "A single block of plaintext";
std::vector<uint8_t> plaintext(std::begin(data), std::end(data));
EncryptResult encryptResult = cryptoClient.Encrypt(EncryptionAlgorithm::RsaOaep, plaintext);
std::cout << " - Encrypted data using the algorithm " << encryptResult.Algorithm.ToString()
        << ",with key " << encryptResult.KeyId << ". The resulting encrypted data is: "
        << Azure::Core::Convert::Base64Encode(encryptResult.Ciphertext) << std::endl;
```

## Decrypting a key

Now decrypt the encrypted data. Note that the same algorithm must always be used for both encrypt and decrypt.

```cpp
DecryptResult decryptResult
    = cryptoClient.Decrypt(EncryptionAlgorithm::RsaOaep, encryptResult.Ciphertext);
std::cout << " - Decrypted data using the algorithm " << decryptResult.Algorithm.ToString()
        << ", with key " << decryptResult.KeyId << ". The resulting decrypted data is: "
        << std::string(decryptResult.Plaintext.begin(), decryptResult.Plaintext.end())
        << std::endl;
```

## Source

To see the full example source, see:

- sample4_encrypt_decrypt.cpp