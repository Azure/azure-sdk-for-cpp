// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  namespace Details {
    constexpr static const char* EcValue = "EC";
    constexpr static const char* EcHsmValue = "EC-HSM";
    constexpr static const char* RsaValue = "RSA";
    constexpr static const char* RsaHsmValue = "RSA-HSM";
    constexpr static const char* OctValue = "oct";
    constexpr static const char* OctHsmValue = "oct-HSM";
  } // namespace Details

  enum class KeyTypeEnum
  {
    Ec,
    EcHsm,
    Rsa,
    RsaHsm,
    Oct,
    OctHsm,
  };

  namespace Details {
    KeyTypeEnum KeyTypeFromString(std::string const& name);
    std::string KeyTypeToString(KeyTypeEnum kty);
  } // namespace Details

}}}} // namespace Azure::Security::KeyVault::Keys
