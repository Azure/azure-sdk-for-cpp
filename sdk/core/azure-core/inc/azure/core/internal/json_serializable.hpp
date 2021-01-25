// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Internal { namespace Json {

  class JsonSerializable {
  public:
    virtual std::string Serialize() const = 0;
  };

}}}} // namespace Azure::Core::Internal::Json
