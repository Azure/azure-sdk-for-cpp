// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <internal/contract.hpp>
#include "http/request.hpp"

using namespace Azure::Core::Http;

BodyStream* BodyStream::null = nullptr;
BodyBuffer* BodyBuffer::null = nullptr;
