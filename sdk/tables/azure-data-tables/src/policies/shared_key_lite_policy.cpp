// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../private/policies/shared_key_lite_policy.hpp"

#include "../private/hmacsha256.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/strings.hpp>

#include <algorithm>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {
}}}}} // namespace Azure::Data::Tables::_detail::Policies
