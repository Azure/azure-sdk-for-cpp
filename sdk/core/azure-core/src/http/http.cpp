// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <internal/contract.hpp>
#include "http/httprequest.hpp"

using namespace azure::core::http;

BodyStream* BodyStream::null = nullptr;
BodyBuffer* BodyBuffer::null = nullptr;
