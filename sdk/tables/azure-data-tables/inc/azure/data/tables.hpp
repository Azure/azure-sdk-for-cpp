// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Includes all public headers from Azure Data Tables SDK library.
 *
 */

#pragma once

#include "azure/data/tables/dll_import_export.hpp"
#include "azure/data/tables/models.hpp"
#include "azure/data/tables/rtti.hpp"
#include "azure/data/tables/serializers.hpp"
#include "azure/data/tables/tables_clients.hpp"
#include "azure/data/tables/transactions.hpp"

#include "azure/data/tables/credentials/shared_key_credential.hpp"
#include "azure/data/tables/policies/service_version_policy.hpp"
#include "azure/data/tables/policies/shared_key_lite_policy.hpp"
#include "azure/data/tables/policies/shared_key_policy.hpp"
#include "azure/data/tables/policies/switch_to_secondary_policy.hpp"
#include "azure/data/tables/policies/tenant_bearer_token_policy.hpp"
#include "azure/data/tables/policies/timeout_policy.hpp"