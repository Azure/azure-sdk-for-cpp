// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Storage { namespace _internal {

  bool IsDirectory(const std::string& path);
  bool IsRegularFile(const std::string& path);
  bool PathExists(const std::string& path);
  void CreateDirectory(const std::string& path);
  void Rename(const std::string& oldPath, const std::string& newPath);
  void Remove(const std::string& path);
  int64_t GetFileSize(const std::string& path);

  class DirectoryIterator final {
  public:
    struct DirectoryEntry final
    {
      std::string Name;
      bool IsDirectory = false;
      int64_t Size = -1;
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

  class MemoryMap final {
  public:
    explicit MemoryMap(const std::string& filename);
    MemoryMap(const MemoryMap&) = delete;
    MemoryMap(MemoryMap&& other) noexcept { *this = std::move(other); }
    MemoryMap& operator=(const MemoryMap&) = delete;
    MemoryMap& operator=(MemoryMap&& other) noexcept;
    ~MemoryMap();

    void* Map(size_t offset, size_t size);

  private:
    void* m_fileHandle = nullptr;
    std::vector<std::pair<void*, size_t>> m_mapped;
  };

}}} // namespace Azure::Storage::_internal
