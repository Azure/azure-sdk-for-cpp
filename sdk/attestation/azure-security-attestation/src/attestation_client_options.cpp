// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/attestation/attestation_client_options.hpp"

namespace Azure { namespace Security { namespace Attestation {
  const ServiceVersion ServiceVersion::V2020_10_01("2020-10-01");

  const AttestationDataType AttestationDataType ::Binary("Binary");
  const AttestationDataType AttestationDataType::Json("Json");

}}} // namespace Azure::Security::Attestation
