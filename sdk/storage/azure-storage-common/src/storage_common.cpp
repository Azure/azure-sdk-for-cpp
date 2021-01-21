// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_common.hpp"

#include <azure/core/uuid.hpp>

#include "azure/storage/common/crypt.hpp"

namespace Azure { namespace Storage {

  std::string CreateUniqueLeaseId()
  {
    auto uuid = Azure::Core::Uuid::CreateUuid();
    return uuid.GetUuidString();
  }

  namespace Details {
    ContentHash FromBase64String(const std::string& base64String, HashAlgorithm algorithm)
    {
      ContentHash hash;
      hash.Value = Azure::Core::Base64Decode(base64String);
      hash.Algorithm = algorithm;
      return hash;
    }

    std::string ToBase64String(const ContentHash& hash)
    {
      return Azure::Core::Base64Encode(hash.Value);
    }
  } // namespace Details

}} // namespace Azure::Storage
