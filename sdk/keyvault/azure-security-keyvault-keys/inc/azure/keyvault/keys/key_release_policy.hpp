// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Defines the KeyReleasePolicy.
 *
 */

#pragma once

#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  namespace Details {
    constexpr static const char* ContentTypePropertyName = "contentType";
    constexpr static const char* DataPropertyName = "data";
  } // namespace Details

  struct KeyReleasePolicy
  {
    std::string ContentType;
    std::vector<uint8_t> Data;

    KeyReleasePolicy() {}

    KeyReleasePolicy(std::vector<uint8_t> data) : Data(std::move(data)) {}
  };

}}}} // namespace Azure::Security::KeyVault::Keys
