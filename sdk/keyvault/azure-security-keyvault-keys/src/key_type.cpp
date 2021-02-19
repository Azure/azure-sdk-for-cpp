// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

#include <stdexcept>

using namespace Azure::Security::KeyVault::Keys;

Kty Details::KeyTypeFromString(std::string const& name)
{
  if (name == EcValue)
  {
    return Kty::Ec;
  }
  if (name == EcHsmValue)
  {
    return Kty::EcHsm;
  }
  if (name == OctValue)
  {
    return Kty::Oct;
  }
  if (name == OctHsmValue)
  {
    return Kty::OctHsm;
  }
  if (name == RsaValue)
  {
    return Kty::Rsa;
  }
  if (name == RsaHsmValue)
  {
    return Kty::RsaHsm;
  }
  throw std::runtime_error("cannot convert " + name + " to key type (kty)");
}

std::string Details::KeyTypeToString(Kty kty)
{
  if (kty == Kty::Ec)
  {
    return EcValue;
  }
  if (kty == Kty::EcHsm)
  {
    return EcHsmValue;
  }
  if (kty == Kty::Oct)
  {
    return OctValue;
  }
  if (kty == Kty::OctHsm)
  {
    return OctHsmValue;
  }
  if (kty == Kty::Rsa)
  {
    return RsaValue;
  }
  if (kty == Kty::RsaHsm)
  {
    return RsaHsmValue;
  }
  return std::string();
}
