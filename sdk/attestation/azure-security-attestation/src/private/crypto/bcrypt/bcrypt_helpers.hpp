// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if 0 // NOTFUNCTIONAL
#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

      namespace _details {
      struct BCryptException : public std::runtime_error
      {
        BCryptException(std::string const& what);
      };
    } // namespace _details
}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
#endif // NOTFUNCTIONAL
