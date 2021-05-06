// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Options for #Azure::Core::Credentials::TokenCredential.
 */

#pragma once

#include "azure/core/internal/client_options.hpp"

namespace Azure { namespace Core { namespace Credentials {
  /**
   * @brief Defines options for #Azure::Core::Credentials::TokenCredential.
   */
  struct TokenCredentialOptions : public Azure::Core::_internal::ClientOptions
  {
  };
}}} // namespace Azure::Core::Credentials
