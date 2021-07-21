// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Keyvault Secrets Client definition.
 *
 */
#include "azure/keyvault/secrets/secret_client.hpp"

#include "private/package_version.hpp"

#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

std::string SecretClient::ClientVersion() const { return PackageVersion::ToString(); }
