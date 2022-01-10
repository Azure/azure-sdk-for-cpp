// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Security { namespace Attestation {

  class AttestationClient final {
  public:
    std::string ClientVersion() const;
    int GetValue(int key) const;
  };

}}} // namespace Azure::Security::Attestation
