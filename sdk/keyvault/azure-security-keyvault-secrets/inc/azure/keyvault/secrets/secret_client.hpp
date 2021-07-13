// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  class SecretClient final {
  public:
    std::string ClientVersion() const;
  };

}}}} // namespace Azure::Security::KeyVault::Keys
