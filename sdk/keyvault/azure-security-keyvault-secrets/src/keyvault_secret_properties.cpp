// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
/**
 * @file
 * @brief Declares SecretProperties.
 *
 */

#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include "private/secret_serializers.hpp"

using namespace Azure::Security::KeyVault::Secrets;

SecretProperties SecretProperties::CreateFromURL(std::string const& url)
{
  // create a url object to validate the string is valid as url
  Azure::Core::Url urlInstance(url);
  SecretProperties result;
  // parse the url into the result object
  _detail::SecretSerializer::ParseIDUrl(result, urlInstance.GetAbsoluteUrl());
  return result;
}
