// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <stdexcept>

using namespace Azure::Security::KeyVault::Keys;

JsonWebKeyType Details::KeyTypeFromString(std::string const& name)
{
  if (name == EcValue)
  {
    return JsonWebKeyType::Ec;
  }
  if (name == EcHsmValue)
  {
    return JsonWebKeyType::EcHsm;
  }
  if (name == OctValue)
  {
    return JsonWebKeyType::Oct;
  }
  if (name == OctHsmValue)
  {
    return JsonWebKeyType::OctHsm;
  }
  if (name == RsaValue)
  {
    return JsonWebKeyType::Rsa;
  }
  if (name == RsaHsmValue)
  {
    return JsonWebKeyType::RsaHsm;
  }
  throw std::runtime_error("cannot convert " + name + " to key type (kty)");
}

std::string Details::KeyTypeToString(JsonWebKeyType kty)
{
  if (kty == JsonWebKeyType::Ec)
  {
    return EcValue;
  }
  if (kty == JsonWebKeyType::EcHsm)
  {
    return EcHsmValue;
  }
  if (kty == JsonWebKeyType::Oct)
  {
    return OctValue;
  }
  if (kty == JsonWebKeyType::OctHsm)
  {
    return OctHsmValue;
  }
  if (kty == JsonWebKeyType::Rsa)
  {
    return RsaValue;
  }
  if (kty == JsonWebKeyType::RsaHsm)
  {
    return RsaHsmValue;
  }
  return std::string();
}
