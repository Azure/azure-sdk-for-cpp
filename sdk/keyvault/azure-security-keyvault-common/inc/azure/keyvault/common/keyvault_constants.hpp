// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provide string constants used for all the Key Vault services.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Common { namespace Details {
  /***************** KeyVault headers *****************/
  static constexpr char const ContentType[] = "content-type";
  static constexpr char const ApplicationJson[] = "application/json";
  static constexpr char const Accept[] = "accept";
  static constexpr char const MsRequestId[] = "x-ms-request-id";
  static constexpr char const MsClientRequestId[] = "x-ms-client-request-id";

  /**************** KeyVault QueryParameters *********/
  static constexpr char const ApiVersion[] = "api-version";
}}}}} // namespace Azure::Security::KeyVault::Common::Details
