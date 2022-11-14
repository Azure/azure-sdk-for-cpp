//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal wrapper layer on top of a uint_8 array.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {

  class KeyBackup final : public Azure::Core::Json::_internal::JsonSerializable {
  public:
    std::vector<uint8_t> Value;

    std::string Serialize() const override;

    static KeyBackup Deserialize(Azure::Core::Http::RawResponse const& rawResponse);
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::_detail
