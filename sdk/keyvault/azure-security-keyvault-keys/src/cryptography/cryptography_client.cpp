// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
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
    result = m_provider->Encrypt(parameters, context);
  }

  return result;
}
