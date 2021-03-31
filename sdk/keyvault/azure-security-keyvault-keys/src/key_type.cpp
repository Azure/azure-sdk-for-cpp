// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <stdexcept>

using namespace Azure::Security::KeyVault::Keys;

JsonWebKeyType KeyType::KeyTypeFromString(std::string const& name)
{
  if (name == _detail::EcValue)
  {
    return JsonWebKeyType::Ec;
  }
  if (name == _detail::EcHsmValue)
  {
    return JsonWebKeyType::EcHsm;
  }
  if (name == _detail::OctValue)
  {
    return JsonWebKeyType::Oct;
  }
  if (name == _detail::OctHsmValue)
  {
    return JsonWebKeyType::OctHsm;
  }
  if (name == _detail::RsaValue)
  {
    return JsonWebKeyType::Rsa;
  }
  if (name == _detail::RsaHsmValue)
  {
    return JsonWebKeyType::RsaHsm;
  }
  throw std::runtime_error("cannot convert " + name + " to key type (kty)");
}

std::string KeyType::KeyTypeToString(JsonWebKeyType kty)
{
  if (kty == JsonWebKeyType::Ec)
  {
    return _detail::EcValue;
  }
  if (kty == JsonWebKeyType::EcHsm)
  {
    return _detail::EcHsmValue;
  }
  if (kty == JsonWebKeyType::Oct)
  {
    return _detail::OctValue;
  }
  if (kty == JsonWebKeyType::OctHsm)
  {
    return _detail::OctHsmValue;
  }
  if (kty == JsonWebKeyType::Rsa)
  {
    return _detail::RsaValue;
  }
  if (kty == JsonWebKeyType::RsaHsm)
  {
    return _detail::RsaHsmValue;
  }
  return std::string();
}
