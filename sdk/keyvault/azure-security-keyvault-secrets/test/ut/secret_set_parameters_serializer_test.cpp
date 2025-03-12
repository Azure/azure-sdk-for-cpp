// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/secret_serializers.hpp"
#include "azure/core/internal/json/json.hpp"
#include "azure/core/internal/json/json_optional.hpp"
#include "azure/core/internal/json/json_serializable.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"
#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"
#include "secret_get_client_deserialize_test.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;


