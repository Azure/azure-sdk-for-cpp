// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_client.hpp"

#include "private/package_version.hpp"

#include <string>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::_detail;

std::string AttestationClient::ClientVersion() const { return PackageVersion::ToString(); }

int AttestationClient::GetValue(int key) const
{
  if (key < 0)
  {
    return 0;
  }

  return key + 1;
}
