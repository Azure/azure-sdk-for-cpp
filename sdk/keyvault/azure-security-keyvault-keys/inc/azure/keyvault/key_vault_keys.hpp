// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Includes all public headers from Azure Key Vault Keys SDK library.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encryption_algorithm.hpp"
#include "azure/keyvault/keys/cryptography/key_wrap_algorithm.hpp"
#include "azure/keyvault/keys/cryptography/sign_result.hpp"
#include "azure/keyvault/keys/cryptography/signature_algorithm.hpp"
#include "azure/keyvault/keys/cryptography/unwrap_result.hpp"
#include "azure/keyvault/keys/cryptography/verify_result.hpp"
#include "azure/keyvault/keys/cryptography/wrap_result.hpp"
#include "azure/keyvault/keys/delete_key_operation.hpp"
#include "azure/keyvault/keys/deleted_key.hpp"
#include "azure/keyvault/keys/dll_import_export.hpp"
#include "azure/keyvault/keys/import_key_options.hpp"
#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_client.hpp"
#include "azure/keyvault/keys/key_client_options.hpp"
#include "azure/keyvault/keys/key_create_options.hpp"
#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_properties.hpp"
#include "azure/keyvault/keys/key_type.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"
#include "azure/keyvault/keys/list_keys_responses.hpp"
#include "azure/keyvault/keys/recover_deleted_key_operation.hpp"
