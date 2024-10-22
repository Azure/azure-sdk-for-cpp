// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/azure_assert.hpp>

#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Cryptography {
  class HmacSha256 final {
  public:
    static std::vector<uint8_t> Compute(
        const std::vector<uint8_t>& data,
        const std::vector<uint8_t>& key);
  };
}}}}} // namespace Azure::Data::Tables::_detail::Cryptography
