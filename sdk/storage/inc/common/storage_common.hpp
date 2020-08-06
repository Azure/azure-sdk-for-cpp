// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Storage {

  template <class... T> void unused(T&&...) {}

  constexpr int32_t c_InfiniteLeaseDuration = -1;

  std::string CreateUniqueLeaseId();

}} // namespace Azure::Storage
