// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_parser.hpp"

namespace Azure { namespace Identity { namespace _detail {
  class TokenParser final {
    TokenParser() = delete;
    ~TokenParser() = delete;

  public:
    static Core::Credentials::AccessToken Parse(
        std::string const& input,
        std::string const& tokenPropertyName,
        std::string const& expirationPropertyName,
        bool expirationReplesentsSecondsFromNow);
  };
}}} // namespace Azure::Identity::_detail
