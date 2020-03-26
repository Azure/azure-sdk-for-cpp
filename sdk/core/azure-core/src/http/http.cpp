// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace azure::core::http;

BodyStream* BodyStream::null = nullptr;
BodyBuffer* BodyBuffer::null = nullptr;
