// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Keys Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {
  /***************** KeyVault Key *****************/
  constexpr static const char KeyPropertyName[] = "key";

  /***************** Key Client *****************/
  constexpr static const char KeysPath[] = "keys";
  constexpr static const char DeletedKeysPath[] = "deletedkeys";

  /***************** Key Properties *****************/
  constexpr static const char ManagedPropertyName[] = "managed";
  constexpr static const char AttributesPropertyName[] = "attributes";
  constexpr static const char TagsPropertyName[] = "tags";
  constexpr static const char ReleasePolicyPropertyName[] = "release_policy";
  constexpr static const char EnabledPropertyName[] = "enabled";
  constexpr static const char NbfPropertyName[] = "nbf";
  constexpr static const char ExpPropertyName[] = "exp";
  constexpr static const char CreatedPropertyName[] = "created";
  constexpr static const char UpdatedPropertyName[] = "updated";
  constexpr static const char RecoverableDaysPropertyName[] = "recoverableDays";
  constexpr static const char RecoveryLevelPropertyName[] = "recoveryLevel";

  /***************** Key Request Parameters *****************/
  constexpr static const char KeyTypePropertyName[] = "kty";
  constexpr static const char KeySizePropertyName[] = "key_size";
  constexpr static const char KeyOpsPropertyName[] = "key_ops";
  constexpr static const char CurveNamePropertyName[] = "crv";
  constexpr static const char PublicExponentPropertyName[] = "public_exponent";

  /***************** JsonWebKey *****************/
  constexpr static const char KeyIdPropertyName[] = "kid";
  constexpr static const char NPropertyName[] = "n";
  constexpr static const char EPropertyName[] = "e";
  constexpr static const char DPPropertyName[] = "dp";
  constexpr static const char DQPropertyName[] = "dq";
  constexpr static const char QIPropertyName[] = "qi";
  constexpr static const char PPropertyName[] = "p";
  constexpr static const char QPropertyName[] = "q";
  constexpr static const char XPropertyName[] = "x";
  constexpr static const char YPropertyName[] = "y";
  constexpr static const char DPropertyName[] = "d";
  constexpr static const char KPropertyName[] = "k";
  constexpr static const char TPropertyName[] = "key_hsm";

  /***************** KeyType *****************/
  constexpr static const char EcValue[] = "EC";
  constexpr static const char EcHsmValue[] = "EC-HSM";
  constexpr static const char RsaValue[] = "RSA";
  constexpr static const char RsaHsmValue[] = "RSA-HSM";
  constexpr static const char OctValue[] = "oct";
  constexpr static const char OctHsmValue[] = "oct-HSM";

  /***************** Deleted Key *****************/
  constexpr static const char RecoveryIdPropertyName[] = "recoveryId";
  constexpr static const char DeletedOnPropertyName[] = "deletedDate";
  constexpr static const char ScheduledPurgeDatePropertyName[] = "scheduledPurgeDate";

  /***************** Curve Names *****************/
  constexpr static const char P256Value[] = "P-256";
  constexpr static const char P256KValue[] = "P-256K";
  constexpr static const char P384Value[] = "P-384";
  constexpr static const char P521Value[] = "P-521";

  constexpr static const char P256OidValue[] = "1.2.840.10045.3.1.7";
  constexpr static const char P256KOidValue[] = "1.3.132.0.10";
  constexpr static const char P384OidValue[] = "1.3.132.0.34";
  constexpr static const char P521OidValue[] = "1.3.132.0.35";

  /***************** Import Key  *****************/
  constexpr static const char HsmPropertyName[] = "hsm";

  /***************** Encryption Algorithm *********/
  constexpr static const char Rsa15Value[] = "RSA1_5";
  constexpr static const char RsaOaepValue[] = "RSA-OAEP";
  constexpr static const char RsaOaep256Value[] = "RSA-OAEP-256";
  constexpr static const char A128GcmValue[] = "A128GCM";
  constexpr static const char A192GcmValue[] = "A192GCM";
  constexpr static const char A256GcmValue[] = "A256GCM";
  constexpr static const char A128CbcValue[] = "A128CBC";
  constexpr static const char A192CbcValue[] = "A192CBC";
  constexpr static const char A256CbcValue[] = "A256CBC";
  constexpr static const char A128CbcPadValue[] = "A128CBCPAD";
  constexpr static const char A192CbcPadValue[] = "A192CBCPAD";
  constexpr static const char A256CbcPadValue[] = "A256CBCPAD";
  constexpr static const char A128KWValueValue[] = "A128KW";
  constexpr static const char A192KWValueValue[] = "A192KW";
  constexpr static const char A256KWValueValue[] = "A256KW";

  /***************** Sign / Verify *********/
  constexpr static const char RS256Value[] = "RS256";
  constexpr static const char RS384Value[] = "RS384";
  constexpr static const char RS512Value[] = "RS512";
  constexpr static const char PS256Value[] = "PS256";
  constexpr static const char PS384Value[] = "PS384";
  constexpr static const char PS512Value[] = "PS512";
  constexpr static const char ES256Value[] = "ES256";
  constexpr static const char ES384Value[] = "ES384";
  constexpr static const char ES512Value[] = "ES512";
  constexpr static const char ES256KValue[] = "ES256K";
  constexpr static const char DigestValue[] = "digest";

  /***************** Encrypt *********/
  constexpr static const char AlgorithmValue[] = "alg";
  constexpr static const char ValueParameterValue[] = "value";
  constexpr static const char IvValue[] = "iv";
  constexpr static const char AdditionalAuthenticatedValue[] = "aad";
  constexpr static const char AuthenticationTagValue[] = "tag";
  constexpr static const char EncryptValue[] = "encrypt";
  constexpr static const char DecryptValue[] = "decrypt";
  constexpr static const char WrapKeyValue[] = "wrapKey";
  constexpr static const char UnwrapKeyValue[] = "unwrapKey";
  constexpr static const char SignValue[] = "sign";
  constexpr static const char VerifyValue[] = "verify";
  constexpr static const char ImportValue[] = "import";

  /***************** Service *********/
  constexpr static const char ApiVersionValue[] = "api-version";

  /***************** Rotation Policy *********/
  constexpr static const char IdValue[] = "id";
  constexpr static const char ExpiryTimeValue[] = "expiryTime";
  constexpr static const char LifeTimeActionsValue[] = "lifetimeActions";
  constexpr static const char RotateActionsValue[] = "rotate";
  constexpr static const char NotifyActionsValue[] = "notify";
  constexpr static const char ActionActionsValue[] = "action";
  constexpr static const char TriggerActionsValue[] = "trigger";
  constexpr static const char TypeActionsValue[] = "type";
  constexpr static const char TBEActionsValue[] = "timeBeforeExpiry";
  constexpr static const char TACActionsValue[] = "timeAfterCreate";

  /***************** Get Random Bytes *********/
  constexpr static const char CountPropertiesValue[] = "count";

}}}}} // namespace Azure::Security::KeyVault::Keys::_detail
