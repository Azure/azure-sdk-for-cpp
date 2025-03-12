// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "secret_backup_deserialize_test.hpp"

#include "../src/private/secret_serializers.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Secrets::_test;
using namespace Azure::Security::KeyVault::Secrets::_detail;
using namespace Azure::Core::Json::_internal;
