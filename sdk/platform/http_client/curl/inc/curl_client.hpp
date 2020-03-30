// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <http/http.hpp>

class CurlClient
{
private:
  azure::core::http::Request const& _request;

public:
  CurlClient(azure::core::http::Request const& request) : _request(request) {}

  azure::core::http::Response send();
};
