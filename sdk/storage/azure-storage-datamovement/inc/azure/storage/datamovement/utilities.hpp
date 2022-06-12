// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <initializer_list>
#include <string>

namespace Azure { namespace Storage { namespace _internal {

  constexpr static const char* FileUrlScheme = "file://";

  std::string JoinPath(const std::initializer_list<std::string>& paths);
  template <class... Args> std::string JoinPath(Args&&... paths) { return JoinPath({paths...}); }
  std::string GetPathUrl(const std::string& relativePath);
  std::string GetPathFromUrl(const std::string& fileUrl);
  std::string RemoveSasToken(const std::string& azureStorageUrl);
  std::string ApplySasToken(const std::string& url, const std::string& sasToken);

}}} // namespace Azure::Storage::_internal
