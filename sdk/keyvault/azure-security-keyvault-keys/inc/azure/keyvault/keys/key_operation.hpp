// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  class KeyOperation {
    std::string m_operation;

  public:
    KeyOperation(std::string const& operation) : m_operation(operation) {}

    std::string const& ToString() const { return m_operation; }

    static KeyOperation Encrypt() { return KeyOperation("encrypt"); }
    static KeyOperation Decrypt() { return KeyOperation("decrypt"); }
    static KeyOperation Sign() { return KeyOperation("sign"); }
    static KeyOperation Verify() { return KeyOperation("verify"); }
    static KeyOperation WrapKey() { return KeyOperation("wrapKey"); }
    static KeyOperation UnwrapKey() { return KeyOperation("unwrapKey"); }
    static KeyOperation Import() { return KeyOperation("import"); }
    static KeyOperation Export() { return KeyOperation("export"); }
  };

}}}} // namespace Azure::Security::KeyVault::Keys
