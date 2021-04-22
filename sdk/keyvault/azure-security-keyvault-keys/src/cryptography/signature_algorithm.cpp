// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/cryptography/signature_algorithm.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  const SignatureAlgorithm SignatureAlgorithm::RS256(_detail::RS256Value);
  const SignatureAlgorithm SignatureAlgorithm::RS384(_detail::RS384Value);
  const SignatureAlgorithm SignatureAlgorithm::RS512(_detail::RS512Value);
  const SignatureAlgorithm SignatureAlgorithm::PS256(_detail::PS256Value);
  const SignatureAlgorithm SignatureAlgorithm::PS384(_detail::PS384Value);
  const SignatureAlgorithm SignatureAlgorithm::PS512(_detail::PS512Value);
  const SignatureAlgorithm SignatureAlgorithm::ES256(_detail::ES256Value);
  const SignatureAlgorithm SignatureAlgorithm::ES384(_detail::ES384Value);
  const SignatureAlgorithm SignatureAlgorithm::ES512(_detail::ES512Value);
  const SignatureAlgorithm SignatureAlgorithm::ES256K(_detail::ES256KValue);

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography
