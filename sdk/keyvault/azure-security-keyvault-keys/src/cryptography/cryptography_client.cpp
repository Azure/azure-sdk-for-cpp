// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/azure_assert.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"
#include "azure/keyvault/keys/cryptography/local_cryptography_provider_factory.hpp"
#include "azure/keyvault/keys/key_operation.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace Azure::Security::KeyVault::Keys::Cryptography::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
// 1Mb at a time
const size_t DefaultStreamDigestReadSize = 1024 * 1024;

inline std::vector<uint8_t> CreateDigest(
    SignatureAlgorithm algorithm,
    Azure::Core::IO::BodyStream& data)
{
  // Use heap for the reading buffer.
  auto heapBuffer = std::make_unique<std::vector<uint8_t>>(DefaultStreamDigestReadSize);
  auto* buffer = heapBuffer.get()->data();
  auto hasAlgo = algorithm.GetHashAlgorithm();
  for (uint64_t read = data.Read(buffer, DefaultStreamDigestReadSize); read > 0;
       read = data.Read(buffer, DefaultStreamDigestReadSize))
  {
    hasAlgo->Append(buffer, static_cast<size_t>(read));
  }
  return hasAlgo->Final();
}

inline std::vector<uint8_t> CreateDigest(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& data)
{
  auto hasAlgo = algorithm.GetHashAlgorithm();
  return hasAlgo->Final(data.data(), data.size());
}
} // namespace

void CryptographyClient::Initialize(std::string const&, Azure::Core::Context const& context)
{
  if (m_provider != nullptr)
  {
    return;
  }

  try
  {
    auto key = m_remoteProvider->GetKey(context).Value;
    m_provider = LocalCryptographyProviderFactory::Create(key);
    if (m_provider == nullptr)
    {
      // KeyVaultKeyType is not supported locally. Use remote client.
      m_provider = m_remoteProvider;
      return;
    }
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    if (e.StatusCode == HttpStatusCode::Forbidden)
    {
      m_provider = m_remoteProvider;
    }
    else
    {
      throw;
    }
  }
  catch (...)
  {
    // re-throw any other exception
    throw;
  }
}

CryptographyClient::CryptographyClient(
    std::string const& keyId,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions const& options,
    bool forceRemote)
{
  auto apiVersion = options.Version.ToString();
  m_keyId = keyId;
  m_remoteProvider = std::make_shared<RemoteCryptographyClient>(keyId, credential, options);
  m_pipeline = m_remoteProvider->Pipeline;

  if (forceRemote)
  {
    m_provider = m_remoteProvider;
  }
}

EncryptResult CryptographyClient::Encrypt(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context)
{
  if (m_provider == nullptr)
  {
    // Try to init a local crypto provider after getting the key from the server.
    // If the local provider can't be created, the remote client is used as provider.
    Initialize(KeyOperation::Encrypt.ToString(), context);
  }

  // Default result has empty values.
  EncryptResult result;

  // m_provider can be local or remote, depending on how it was init.
  if (m_provider->SupportsOperation(KeyOperation::Encrypt))
  {
    try
    {
      result = m_provider->Encrypt(parameters, context);
    }
    catch (std::exception const&)
    {
      // If provider supports remote, otherwise re-throw
      if (!m_provider->CanRemote())
      {
        throw;
      }
    }

    if (result.Ciphertext.empty())
    {
      // Operation is not completed yet. Either not supported by the provider or not generated.
      // Assert the client is NOT localOnly before running the operation on Key Vault Service with
      // the remote provider.
      AZURE_ASSERT_FALSE(LocalOnly());

      result = m_remoteProvider->Encrypt(parameters, context);
    }
  }

  return result;
}

DecryptResult CryptographyClient::Decrypt(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context)
{
  if (m_provider == nullptr)
  {
    // Try to init a local crypto provider after getting the key from the server.
    // If the local provider can't be created, the remote client is used as provider.
    Initialize(KeyOperation::Decrypt.ToString(), context);
  }

  // Default result has empty values.
  DecryptResult result;

  // m_provider can be local or remote, depending on how it was init.
  if (m_provider->SupportsOperation(KeyOperation::Decrypt))
  {
    try
    {
      result = m_provider->Decrypt(parameters, context);
    }
    catch (std::exception const&)
    {
      // If provider supports remote, otherwise re-throw
      if (!m_provider->CanRemote())
      {
        throw;
      }
    }

    if (result.Plaintext.empty())
    {
      // Operation is not completed yet. Either not supported by the provider or not generated.
      // Assert the client is NOT localOnly before running the operation on Key Vault Service with
      // the remote provider.
      AZURE_ASSERT_FALSE(LocalOnly());

      result = m_remoteProvider->Decrypt(parameters, context);
    }
  }

  return result;
}

WrapResult CryptographyClient::WrapKey(
    KeyWrapAlgorithm algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context)
{
  if (m_provider == nullptr)
  {
    // Try to init a local crypto provider after getting the key from the server.
    // If the local provider can't be created, the remote client is used as provider.
    Initialize(KeyOperation::WrapKey.ToString(), context);
  }

  // Default result has empty values.
  WrapResult result;

  // m_provider can be local or remote, depending on how it was init.
  if (m_provider->SupportsOperation(KeyOperation::WrapKey))
  {
    try
    {
      result = m_provider->WrapKey(algorithm, key, context);
    }
    catch (std::exception const&)
    {
      // If provider supports remote, otherwise re-throw
      if (!m_provider->CanRemote())
      {
        throw;
      }
    }

    if (result.EncryptedKey.size() == 0)
    {
      // Operation is not completed yet. Either not supported by the provider or not generated.
      // Assert the client is NOT localOnly before running the operation on Key Vault Service with
      // the remote provider.
      AZURE_ASSERT_FALSE(LocalOnly());

      result = m_remoteProvider->WrapKey(algorithm, key, context);
    }
  }

  return result;
}

UnwrapResult CryptographyClient::UnwrapKey(
    KeyWrapAlgorithm algorithm,
    std::vector<uint8_t> const& encryptedKey,
    Azure::Core::Context const& context)
{
  if (m_provider == nullptr)
  {
    // Try to init a local crypto provider after getting the encryptedKey from the server.
    // If the local provider can't be created, the remote client is used as provider.
    Initialize(KeyOperation::UnwrapKey.ToString(), context);
  }

  // Default result has empty values.
  UnwrapResult result;

  // m_provider can be local or remote, depending on how it was init.
  if (m_provider->SupportsOperation(KeyOperation::UnwrapKey))
  {
    try
    {
      result = m_provider->UnwrapKey(algorithm, encryptedKey, context);
    }
    catch (std::exception const&)
    {
      // If provider supports remote, otherwise re-throw
      if (!m_provider->CanRemote())
      {
        throw;
      }
    }

    if (result.Key.size() == 0)
    {
      // Operation is not completed yet. Either not supported by the provider or not generated.
      // Assert the client is NOT localOnly before running the operation on Key Vault Service with
      // the remote provider.
      AZURE_ASSERT_FALSE(LocalOnly());

      result = m_remoteProvider->UnwrapKey(algorithm, encryptedKey, context);
    }
  }

  return result;
}

SignResult CryptographyClient::Sign(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& digest,
    Azure::Core::Context const& context)
{
  if (m_provider == nullptr)
  {
    // Try to init a local crypto provider after getting the encryptedKey from the server.
    // If the local provider can't be created, the remote client is used as provider.
    Initialize(KeyOperation::Sign.ToString(), context);
  }

  // Default result has empty values.
  SignResult result;

  // m_provider can be local or remote, depending on how it was init.
  if (m_provider->SupportsOperation(KeyOperation::Sign))
  {
    try
    {
      result = m_provider->Sign(algorithm, digest, context);
    }
    catch (std::exception const&)
    {
      // If provider supports remote, otherwise re-throw
      if (!m_provider->CanRemote())
      {
        throw;
      }
    }

    if (result.Signature.size() == 0)
    {
      // Operation is not completed yet. Either not supported by the provider or not generated.
      // Assert the client is NOT localOnly before running the operation on Key Vault Service with
      // the remote provider.
      AZURE_ASSERT_FALSE(LocalOnly());

      result = m_remoteProvider->Sign(algorithm, digest, context);
    }
  }

  return result;
}

SignResult CryptographyClient::SignData(
    SignatureAlgorithm algorithm,
    Azure::Core::IO::BodyStream& data,
    Azure::Core::Context const& context)
{
  return Sign(algorithm, CreateDigest(algorithm, data), context);
}

SignResult CryptographyClient::SignData(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& data,
    Azure::Core::Context const& context)
{
  return Sign(algorithm, CreateDigest(algorithm, data), context);
}

VerifyResult CryptographyClient::Verify(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& digest,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context)
{
  if (m_provider == nullptr)
  {
    // Try to init a local crypto provider after getting the encryptedKey from the server.
    // If the local provider can't be created, the remote client is used as provider.
    Initialize(KeyOperation::Verify.ToString(), context);
  }

  // Default result has empty values.
  VerifyResult result;

  // m_provider can be local or remote, depending on how it was init.
  if (m_provider->SupportsOperation(KeyOperation::Verify))
  {
    try
    {
      result = m_provider->Verify(algorithm, digest, signature, context);
    }
    catch (std::exception const&)
    {
      // If provider supports remote, otherwise re-throw
      if (!m_provider->CanRemote())
      {
        throw;
      }
    }

    if (result.KeyId.empty())
    {
      // Operation is not completed yet. Either not supported by the provider or not generated.
      // Assert the client is NOT localOnly before running the operation on Key Vault Service with
      // the remote provider.
      AZURE_ASSERT_FALSE(LocalOnly());

      result = m_remoteProvider->Verify(algorithm, digest, signature, context);
    }
  }

  return result;
}

VerifyResult CryptographyClient::VerifyData(
    SignatureAlgorithm algorithm,
    Azure::Core::IO::BodyStream& data,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context)
{
  return Verify(algorithm, CreateDigest(algorithm, data), signature, context);
}

VerifyResult CryptographyClient::VerifyData(
    SignatureAlgorithm algorithm,
    std::vector<uint8_t> const& data,
    std::vector<uint8_t> const& signature,
    Azure::Core::Context const& context)
{
  return Verify(algorithm, CreateDigest(algorithm, data), signature, context);
}
