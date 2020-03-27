// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <map>
#include <string>
#include <vector>

#include <http/http.hpp>

using namespace azure::core::http;

uint16_t const& Response::getStatusCode() { return _statusCode; }

std::string const& Response::getReasonPhrase() { return _reasonPhrase; }

std::map<std::string, std::string> const& Response::getHeaders() { return this->_headers; }

BodyStream* Response::getBodyStream() { return _bodyStream; }

BodyBuffer* Response::getBodyBuffer() { return _bodyBuffer; }
