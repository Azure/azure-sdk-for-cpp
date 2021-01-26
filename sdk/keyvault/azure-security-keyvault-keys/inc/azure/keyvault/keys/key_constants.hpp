// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Details {
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

}}}}} // namespace Azure::Security::KeyVault::Keys::Details
