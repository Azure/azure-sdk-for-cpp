// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Keyvault Secret serializers
 */

#include "private/secret_serializers.hpp"

#include "private/secret_constants.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

using namespace Azure::Core::_internal;
using namespace Azure::Core::Json::_internal;
using Azure::Core::_internal::PosixTimeConverter;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;

