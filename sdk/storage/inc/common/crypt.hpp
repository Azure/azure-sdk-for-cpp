// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Storage {

  std::string HMAC_SHA256(const std::string& text, const std::string& key);
  std::string Base64Encode(const std::string& text);
  std::string Base64Decode(const std::string& text);

}} // namespace Azure::Storage
