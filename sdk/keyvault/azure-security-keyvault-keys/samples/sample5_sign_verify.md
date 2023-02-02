# Signing and verifying keys

This sample demonstrates how to sign data with both a RSA key and an EC key.
To get started, you'll need a URI to an Azure Key Vault. See the [README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/keyvault/azure-security-keyvault-keys/README.md) for links and instructions.

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

## Creating keys

First, we'll create both an RSA key and an EC key which will be used to sign and verify.

```cpp
auto rsaKeyName = "CloudRsaKey-" + Azure::Core::Uuid::CreateUuid().ToString();
auto keyOptions = CreateRsaKeyOptions(rsaKeyName, false);
keyOptions.KeySize = 2048;

auto ecKeyName = "CloudEcKey-" + Azure::Core::Uuid::CreateUuid().ToString();
auto ecKeyOptions = CreateEcKeyOptions(ecKeyName, false);
ecKeyOptions.CurveName = KeyCurveName::P256K;

KeyVaultKey cloudRsaKey = keyClient.CreateRsaKey(keyOptions).Value;
std::cout << " - Key is returned with name " << cloudRsaKey.Name() << " and type "
        << cloudRsaKey.GetKeyType().ToString() << std::endl;

KeyVaultKey cloudEcKey = keyClient.CreateEcKey(ecKeyOptions).Value;
std::cout << " - Key is returned with name " << cloudEcKey.Name() << " and type "
        << cloudEcKey.GetKeyType().ToString() << std::endl;
```

## Creating CryptographyClients

Then, we create the `CryptographyClient` which can perform cryptographic operations with the key we just created using the same credential created above.

```cpp
CryptographyClient rsaCryptoClient(cloudRsaKey.Id(), credential);

CryptographyClient ecCryptoClient(cloudEcKey.Id(), credential);
```

## Signing keys with the Sign and Verify methods

Next, we'll sign some arbitrary data and verify the signatures using the `CryptographyClient` with both the EC and RSA keys we created.
The `Sign` and `Verify` methods expect a precalculated digest, and the digest needs to be calculated using the hash algorithm which matches the signature algorithm being used.
SHA256 is the hash algorithm used for both RS256 and ES256K which are the algorithms we'll be using in this sample.

```cpp
// digestBase64 simulates some text data that has been hashed using the SHA256 algorithm
// and then base 64 encoded. It is not relevant for the sample how to create the SHA256
// hashed digest.
// Example input data source for the digest:
// "This is some sample data which we will use to demonstrate sign and verify"
std::string digestBase64 = "DU9EdhpwhJqnGnieD0qKYEz6e8QPKlOVpYZZro+XtI8=";
std::vector<uint8_t> digest = Azure::Core::Convert::Base64Decode(digestBase64);

// Sign and Verify from digest
SignResult rsaSignResult = rsaCryptoClient.Sign(SignatureAlgorithm::RS256, digest);
std::cout << " - Signed digest using the algorithm " << rsaSignResult.Algorithm.ToString()
        << ", with key " << rsaSignResult.KeyId << ". The resulting signature is: "
        << Azure::Core::Convert::Base64Encode(rsaSignResult.Signature) << std::endl;

SignResult ecSignResult = ecCryptoClient.Sign(SignatureAlgorithm::ES256K, digest);
std::cout << " - Signed digest using the algorithm " << ecSignResult.Algorithm.ToString()
        << ", with key " << ecSignResult.KeyId << ". The resulting signature is: "
        << Azure::Core::Convert::Base64Encode(ecSignResult.Signature) << std::endl;
```

## Verifying signatures

Verify the digest by comparing the signature you created previously.

```cpp
VerifyResult rsaVerifyResult
    = rsaCryptoClient.Verify(SignatureAlgorithm::RS256, digest, rsaSignResult.Signature);
std::cout << " - Verified the signature using the algorithm "
        << rsaVerifyResult.Algorithm.ToString() << ", with key " << rsaVerifyResult.KeyId
        << ". Signature is valid: " << (rsaVerifyResult.IsValid ? "True" : "False")
        << std::endl;

VerifyResult ecVerifyResult
    = ecCryptoClient.Verify(SignatureAlgorithm::ES256K, digest, ecSignResult.Signature);
std::cout << " - Verified the signature using the algorithm "
        << ecVerifyResult.Algorithm.ToString() << ", with key " << ecVerifyResult.KeyId
        << ". Signature is valid: " << (ecVerifyResult.IsValid ? "True" : "False") << std::endl;
```

## Signing keys with the SignData and VerifyData methods

The `SignData` and `VerifyData` methods take the raw data which is to be signed. The calculate the digest for the user so there is no need to compute the digest.

```cpp
SignResult rsaSignDataResult = rsaCryptoClient.SignData(SignatureAlgorithm::RS256, data);
std::cout << " - Signed data using the algorithm " << rsaSignDataResult.Algorithm.ToString()
        << ", with key " << rsaSignDataResult.KeyId << ". The resulting signature is: "
        << Azure::Core::Convert::Base64Encode(rsaSignDataResult.Signature) << std::endl;

SignResult ecSignDataResult = ecCryptoClient.SignData(SignatureAlgorithm::ES256K, data);
std::cout << " - Signed data using the algorithm " << ecSignDataResult.Algorithm.ToString()
        << ", with key " << ecSignDataResult.KeyId << ". The resulting signature is: "
        << Azure::Core::Convert::Base64Encode(ecSignDataResult.Signature) << std::endl;
```

## Verifying signatures with VerifyData methods

You can provide the same data for which you generated a signature above to `VerifyData` to generate and compare the digest. To be valid, the generated digest must match the given signature.

```cpp
VerifyResult rsaVerifyDataResult
    = rsaCryptoClient.VerifyData(SignatureAlgorithm::RS256, data, rsaSignDataResult.Signature);
std::cout << " - Verified the signature using the algorithm "
        << rsaVerifyDataResult.Algorithm.ToString() << ", with key "
        << rsaVerifyDataResult.KeyId
        << ". Signature is valid: " << (rsaVerifyDataResult.IsValid ? "True" : "False")
        << std::endl;

VerifyResult ecVerifyDataResult
    = ecCryptoClient.VerifyData(SignatureAlgorithm::ES256K, data, ecSignDataResult.Signature);
std::cout << " - Verified the signature using the algorithm "
        << ecVerifyDataResult.Algorithm.ToString() << ", with key " << ecVerifyDataResult.KeyId
        << ". Signature is valid: " << (ecVerifyDataResult.IsValid ? "True" : "False")
        << std::endl;
```

## Source

To see the full example source, see:

- sample5_sign_verify.cpp
