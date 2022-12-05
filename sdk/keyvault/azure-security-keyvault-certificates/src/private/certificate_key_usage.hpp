// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Key Usage constants.
 *
 */
#pragma once
#include <azure/keyvault/certificates/certificate_client_models.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {

  const CertificateKeyUsage CertificateKeyUsage::DigitalSignature(_detail::DigitalSignatureValue);
  const CertificateKeyUsage CertificateKeyUsage::NonRepudiation(_detail::NonRepudiationValue);
  const CertificateKeyUsage CertificateKeyUsage::KeyEncipherment(_detail::KeyEnciphermentValue);
  const CertificateKeyUsage CertificateKeyUsage::DataEncipherment(_detail::DataEnciphermentValue);
  const CertificateKeyUsage CertificateKeyUsage::KeyAgreement(_detail::KeyAgreementValue);
  const CertificateKeyUsage CertificateKeyUsage::KeyCertSign(_detail::KeyCertSignValue);
  const CertificateKeyUsage CertificateKeyUsage::CrlSign(_detail::CrlSignValue);
  const CertificateKeyUsage CertificateKeyUsage::EncipherOnly(_detail::EncipherOnlyValue);
  const CertificateKeyUsage CertificateKeyUsage::DecipherOnly(_detail::DecipherOnlyValue);

  const CertificateKeyType CertificateKeyType::Ec(_detail::EcValue);
  const CertificateKeyType CertificateKeyType::EcHsm(_detail::EcHsmValue);
  const CertificateKeyType CertificateKeyType::Rsa(_detail::RsaValue);
  const CertificateKeyType CertificateKeyType::RsaHsm(_detail::RsaHsmValue);

  const CertificateKeyCurveName CertificateKeyCurveName::P256(_detail::P256Value);
  const CertificateKeyCurveName CertificateKeyCurveName::P256K(_detail::P256KValue);
  const CertificateKeyCurveName CertificateKeyCurveName::P384(_detail::P384Value);
  const CertificateKeyCurveName CertificateKeyCurveName::P521(_detail::P521Value);

  const CertificateContentType CertificateContentType::Pkcs12(_detail::Pkc12Value);
  const CertificateContentType CertificateContentType::Pem(_detail::PemValue);

  const CertificatePolicyAction CertificatePolicyAction::AutoRenew(_detail::AutoRenewValue);
  const CertificatePolicyAction CertificatePolicyAction::EmailContacts(_detail::EmailContactsValue);

}}}} // namespace Azure::Security::KeyVault::Certificates