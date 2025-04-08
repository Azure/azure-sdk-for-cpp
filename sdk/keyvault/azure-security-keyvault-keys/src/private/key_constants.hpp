// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Keys Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace _detail {
  constexpr static const char KeyVaultServicePackageName[] = "keyvault-keys";

  /***************** KeyVault Key *****************/
  //constexpr static const char KeyPropertyName[] = "key";

  /***************** Key Client *****************/
  constexpr static const char KeysPath[] = "keys";
  //constexpr static const char DeletedKeysPath[] = "deletedkeys";

  /***************** Key Properties *****************/
  constexpr static const char TagsPropertyName[] = "tags";
  
  
  /***************** JsonWebKey *****************/
  constexpr static const char KeyIdPropertyName[] = "kid";

  /***************** KeyType *****************/
  constexpr static const char EcValue[] = "EC";
  constexpr static const char EcHsmValue[] = "EC-HSM";
  constexpr static const char RsaValue[] = "RSA";
  constexpr static const char RsaHsmValue[] = "RSA-HSM";
  constexpr static const char OctValue[] = "oct";
  constexpr static const char OctHsmValue[] = "oct-HSM";

  
  /***************** Curve Names *****************/
  constexpr static const char P256Value[] = "P-256";
  constexpr static const char P256KValue[] = "P-256K";
  constexpr static const char P384Value[] = "P-384";
  constexpr static const char P521Value[] = "P-521";

  constexpr static const char P256OidValue[] = "1.2.840.10045.3.1.7";
  constexpr static const char P256KOidValue[] = "1.3.132.0.10";
  constexpr static const char P384OidValue[] = "1.3.132.0.34";
  constexpr static const char P521OidValue[] = "1.3.132.0.35";

  
  /***************** Encryption Algorithm *********/
  // cspell: ignore CBCPAD
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
  constexpr static const char ExportValue[] = "export";

  /***************** Service *********/
  constexpr static const char ApiVersionValue[] = "api-version";


  /***************** Get Random Bytes *********/
  constexpr static const char CountPropertiesValue[] = "count";

  /***************** Release *********/
  constexpr static const char CKM_RSA_AES_KEY_WRAP_Value[] = "CKM_RSA_AES_KEY_WRAP";
  constexpr static const char RSA_AES_KEY_WRAP_256_Value[] = "RSA_AES_KEY_WRAP_256";
  constexpr static const char RSA_AES_KEY_WRAP_384_Value[] = "RSA_AES_KEY_WRAP_384";
}}}}} // namespace Azure::Security::KeyVault::Keys::_detail
