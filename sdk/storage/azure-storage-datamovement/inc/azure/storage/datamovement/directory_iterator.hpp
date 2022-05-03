// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {

  class DirectoryIterator {
  public:
    struct DirectoryEntry
    {
      std::string Name;
      bool IsDirectory = false;
    };
    explicit DirectoryIterator(const std::string& rootDirectory);
    DirectoryIterator(const DirectoryIterator&) = delete;
    DirectoryIterator& operator=(const DirectoryIterator&) = delete;
    ~DirectoryIterator();

    DirectoryEntry Next();

  private:
    std::string m_rootDirectory;
    void* m_directroyObject;
  };

}}}} // namespace Azure::Storage::DataMovement::_internal
