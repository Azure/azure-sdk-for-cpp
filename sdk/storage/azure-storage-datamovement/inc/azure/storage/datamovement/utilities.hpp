// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Storage { namespace _internal {

  std::string GetFileUrl(const std::string& relativePath);

}}} // namespace Azure::Storage::_internal
