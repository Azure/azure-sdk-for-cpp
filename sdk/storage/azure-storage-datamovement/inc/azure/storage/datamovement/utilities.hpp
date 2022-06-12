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
  std::string RemoveSasToken(const std::string& url);
  std::string ApplySasToken(const std::string& url, const std::string& sasToken);

  // TODO: memory order
  int64_t AtomicFetchAdd(int64_t* arg, int64_t value);
  int64_t AtomicLoad(int64_t* arg);

  template <class T> class MovablePtr {
  public:
    constexpr MovablePtr() noexcept {}
    constexpr MovablePtr(std::nullptr_t) noexcept {}
    explicit MovablePtr(T* ptr) noexcept : m_pointer(ptr) {}
    
    MovablePtr(const MovablePtr<T>&) noexcept  = default;
    MovablePtr(MovablePtr<T>&& other) noexcept
    {
      m_pointer = other.m_pointer;
      other.m_pointer = nullptr;
    }
    MovablePtr<T>& operator=(T* ptr) noexcept
    {
      m_pointer = ptr;
      return *this;
    }
    MovablePtr& operator=(const MovablePtr<T>&) noexcept = default;
    MovablePtr<T>& operator=(MovablePtr<T>&& other) noexcept
    {
      m_pointer = other.m_pointer;
      other.m_pointer = nullptr;
      return *this;
    }

    explicit operator bool() const noexcept { return bool(m_pointer); }

    T* operator->() const noexcept { return m_pointer; }
    T& operator*() const noexcept { return *m_pointer; }

  private:
    T* m_pointer = nullptr;
  };

}}} // namespace Azure::Storage::_internal
