// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_common.hpp"

#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage { namespace _internal {
  ContentHash FromBase64String(const std::string& base64String, HashAlgorithm algorithm)
  {
    ContentHash hash;
    hash.Value = Azure::Core::Convert::Base64Decode(base64String);
    hash.Algorithm = algorithm;
    return hash;
  }

  std::string ToBase64String(const ContentHash& hash)
  {
    return Azure::Core::Convert::Base64Encode(hash.Value);
  }
}}} // namespace Azure::Storage::_internal