// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Storage {

  template <class... T> void unused(T&&...) {}

  constexpr int32_t InfiniteLeaseDuration = -1;
  constexpr static const char* AccountEncryptionKey = "$account-encryption-key";
  constexpr static const char* ETagWildcard = "*";

  std::string CreateUniqueLeaseId();

}} // namespace Azure::Storage
