// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/key_constants.hpp"

#include <stdexcept>

using namespace Azure::Security::KeyVault::Keys;

KeyTypeEnum Details::KeyTypeFromString(std::string const& name)
{
  if (name == EcValue)
  {
    return KeyTypeEnum::Ec;
  }
  if (name == EcHsmValue)
  {
    return KeyTypeEnum::EcHsm;
  }
  if (name == OctValue)
  {
    return KeyTypeEnum::Oct;
  }
  if (name == OctHsmValue)
  {
    return KeyTypeEnum::OctHsm;
  }
  if (name == RsaValue)
  {
    return KeyTypeEnum::Rsa;
  }
  if (name == RsaHsmValue)
  {
    return KeyTypeEnum::RsaHsm;
  }
  throw std::runtime_error("cannot convert " + name + " to key type (kty)");
}

std::string Details::KeyTypeToString(KeyTypeEnum kty)
{
  if (kty == KeyTypeEnum::Ec)
  {
    return EcValue;
  }
  if (kty == KeyTypeEnum::EcHsm)
  {
    return EcHsmValue;
  }
  if (kty == KeyTypeEnum::Oct)
  {
    return OctValue;
  }
  if (kty == KeyTypeEnum::OctHsm)
  {
    return OctHsmValue;
  }
  if (kty == KeyTypeEnum::Rsa)
  {
    return RsaValue;
  }
  if (kty == KeyTypeEnum::RsaHsm)
  {
    return RsaHsmValue;
  }
  return std::string();
}
