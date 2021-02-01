// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the KeyTypeEnum.
 *
 */

#pragma once

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

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
