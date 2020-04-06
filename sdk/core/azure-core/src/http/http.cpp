// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace azure::core::http;

BodyStream* BodyStream::null = nullptr;
BodyBuffer* BodyBuffer::null = nullptr;

const std::string HttpVersion::m_title = std::string("HTTP/");
const std::string HttpVersion::m_separator = std::string(".");
