// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/hmacsha256.hpp"
#include "../src/private/policies/shared_key_lite_policy.hpp"
#include "azure/data/tables/credentials/named_key_credential.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <gtest/gtest.h>

using namespace Azure::Core::Http::Policies;
using namespace Azure::Data::Tables::_detail::Policies;
using namespace Azure::Data::Tables::Credentials;
namespace Azure { namespace Data { namespace Tables { namespace _internal { namespace Policies {
  namespace Test {

}}}}}} // namespace Azure::Data::Tables::_internal::Policies::Test
