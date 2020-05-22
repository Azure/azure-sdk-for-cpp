// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>
#include <http/policy.hpp>

using namespace Azure::Core::Http;

Response HttpPolicy::NextPolicy() { return Response(200, "OK\n"); }