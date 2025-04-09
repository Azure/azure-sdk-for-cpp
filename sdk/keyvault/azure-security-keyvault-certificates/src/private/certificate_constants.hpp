// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Certificates Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {
    /***************** Certificates Key Usage *****************/
    constexpr static const char DigitalSignatureValue[] = "digitalSignature";
    constexpr static const char NonRepudiationValue[] = "nonRepudiation";
    constexpr static const char KeyEnciphermentValue[] = "keyEncipherment";
    constexpr static const char DataEnciphermentValue[] = "dataEncipherment";
    constexpr static const char KeyAgreementValue[] = "keyAgreement";
    constexpr static const char KeyCertSignValue[] = "keyCertSign";
    constexpr static const char CrlSignValue[] = "crlSign";
    constexpr static const char EncipherOnlyValue[] = "encipherOnly";
    constexpr static const char DecipherOnlyValue[] = "decipherOnly";

    /***************** Certificates Key Type *****************/
    constexpr static const char EcValue[] = "EC";
    constexpr static const char EcHsmValue[] = "EC-HSM";
    constexpr static const char RsaValue[] = "RSA";
    constexpr static const char RsaHsmValue[] = "RSA-HSM";

    /***************** Certificates Curve Name *****************/
    constexpr static const char P256Value[] = "P-256";
    constexpr static const char P256KValue[] = "P-256K";
    constexpr static const char P384Value[] = "P-384";
    constexpr static const char P521Value[] = "P-521";

    /***************** Certificates Content Type *****************/
    constexpr static const char Pkc12Value[] = "application/x-pkcs12";
    constexpr static const char PemValue[] = "application/x-pem-file";

    /***************** Certificates Policy Action *****************/
    constexpr static const char AutoRenewValue[] = "AutoRenew";
    constexpr static const char EmailContactsValue[] = "EmailContacts";

    /****************** Certificate Operation Properties **********/
    constexpr static const char CompletedValue[] = "completed";
    constexpr static const char DeletedValue[] = "deleted";

}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail
