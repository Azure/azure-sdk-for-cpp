// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace Azure::Core::Http;

std::unique_ptr<BodyStream> BodyStream::null = nullptr;
