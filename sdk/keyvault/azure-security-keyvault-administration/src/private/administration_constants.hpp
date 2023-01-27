// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the string constants used by Key Vault Administration Client.
 *
 */

#pragma once

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
  namespace _detail {
    /***************** Administration moniker *****************/
    constexpr static const char KeyVaultServicePackageName[] = "keyvault-administration";

    /***************** Request components *****************/
    constexpr static const char ContentHeaderName[] = "content-type";
    constexpr static const char ApplicationJsonValue[] = "application/json";
    constexpr static const char ApiVersionQueryParamName[] = "api-version";
    constexpr static const char SettingPathName[] = "settings";

    /***************** JSON components *****************/
    constexpr static const char SettingNodeName[] = "settings";
    constexpr static const char ValueField[] = "value";
    constexpr static const char NameField[] = "name";
    constexpr static const char TypeField[] = "type";
}}}}} // namespace Azure::Security::KeyVault::Administration::_detail
