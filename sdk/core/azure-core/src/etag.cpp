// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/etag.hpp"

using Azure::ETag;

const ETag& ETag::Any()
{
  static ETag any = ETag("*");
  return any;
}