// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include <azure/core/context.hpp>
#include <azure/core/http/raw_response.hpp>

namespace Azure { namespace Storage {

  namespace _internal {
    template <class Derived> class PageResult {
    public:
      std::unique_ptr<Azure::Core::Http::RawResponse> RawResponse;
      std::string NextPageToken;

      bool HasMore() const { return !NextPageToken.empty(); }

      void NextPage()
      {
        if (!HasMore())
        {
          // NextPage() should NEVER be called if HasMore() is false
          std::abort(); // or throw std::out_of_range
        }

        static_assert(
            std::is_base_of<PageResult, Derived>::value,
            "The template argument \"Derived\" should derive from PageResult<Derived>.");

        static_cast<Derived*>(this)->OnNextPage(m_context);
      }

    protected:
      PageResult() = default;
      ~PageResult() = default;
      PageResult(PageResult&&) = default;
      PageResult& operator=(PageResult&&) = default;

      Azure::Core::Context m_context;
    };

  } // namespace _internal

  template <class T> class Pageable {
  public:
    explicit Pageable(T value) : m_value(std::move(value))
    {
      static_assert(
          std::is_base_of<_internal::PageResult<T>, T>::value,
          "The template argument should derive from PageResult");
    }

    class Iterator {
    public:
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = T;
      using pointer = value_type*;
      using reference = value_type&;

      explicit Iterator(pointer ptr) : m_ptr(ptr) {}

      bool operator==(const Iterator& other) const { return m_ptr == other.m_ptr; }
      bool operator!=(const Iterator& other) const { return !(*this == other); }

      Iterator& operator++()
      {
        if (m_ptr->HasMore())
        {
          m_ptr->NextPage();
        }
        else
        {
          m_ptr = nullptr;
        }
        return *this;
      }

      Iterator operator++(int)
      {
        Iterator ret = *this;
        ++(*this);
        return ret;
      }

      reference operator*() const { return *m_ptr; }
      pointer operator->() const { return m_ptr; }

    private:
      pointer m_ptr;
    };

    Iterator begin() { return Iterator(&m_value); }
    Iterator end() { return Iterator(nullptr); }

  private:
    T m_value;
  };

}} // namespace Azure::Storage
