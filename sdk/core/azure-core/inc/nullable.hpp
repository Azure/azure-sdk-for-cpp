// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <new> // for placement new
#include <stdlib.h> // for abort
#include <utility> // for swap and move

namespace Azure { namespace Core {

  template <class T> class Nullable {
    union
    {
      char m_disengaged; // due to constexpr rules for the default constructor
      T m_value;
    };

    bool m_hasValue;

    void Destroy() noexcept /* enforces termination */
    {
      if (m_hasValue)
      {
        m_value.~T();
        m_hasValue = false;
      }
    }

  public:
    constexpr Nullable() : m_disengaged{}, m_hasValue(false) {}
    constexpr Nullable(const T& initialValue) : m_value(initialValue), m_hasValue(true) {}

    Nullable(const Nullable& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
        : m_hasValue(other.m_hasValue)
    {
      if (m_hasValue)
      {
        // standard implementation has to use std::addressof instead of & :)
        ::new (static_cast<void*>(&m_value)) T(other.m_value);
      }
    }

    Nullable(Nullable&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_hasValue(other.m_hasValue)
    {
      if (m_hasValue)
      {
        // standard implementation has to use std::addressof instead of & :)
        ::new (static_cast<void*>(&m_value)) T(std::move(other.m_value));
        other.m_hasValue = false;
      }
    }

    ~Nullable()
    {
      if (m_hasValue)
      {
        m_value.~T();
      }
    }

    // this assumes that swap can't throw if T is nothrow move constructible because
    // is_nothrow_swappable is added in C++17
    void Swap(Nullable& other) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
      if (m_hasValue)
      {
        if (other.m_hasValue)
        {
          using std::swap;
          swap(m_value, other.m_value);
        }
        else
        {
          ::new (static_cast<void*>(&other.m_value)) T(std::move(m_value)); // throws
          other.m_hasValue = true;
          Destroy();
        }
      }
      else if (other.m_hasValue)
      {
        ::new (static_cast<void*>(&m_value)) T(std::move(other.m_value)); // throws
        m_hasValue = true;
        other.Destroy();
      }
    }

    //Intentionally lowercase to override the swap
    friend void swap(Nullable& lhs, Nullable& rhs) noexcept(
        std::is_nothrow_move_constructible<T>::value)
    {
      lhs.Swap(rhs);
    }

    Nullable& operator=(const Nullable& other)
    {
      // this copy and swap may be inefficient for some Ts but
      // it's a lot less code than the standard implementation :)
      Nullable{other}.Swap(*this);
      return *this;
    }

    Nullable& operator=(Nullable&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
      // this move and swap may be inefficient for some Ts but
      // it's a lot less code than the standard implementation :)
      Nullable{std::move(other)}.Swap(*this);
      return *this;
    }

    bool HasValue() const noexcept { return m_hasValue; }

    const T& GetValue() const& noexcept
    {
      if (!m_hasValue)
      {
        // throwing here prohibited by our guidelines
        // https://azure.github.io/azure-sdk/cpp_design.html#pre-conditions
        std::abort();
      }

      return m_value;
    }

    T& GetValue() & noexcept
    {
      if (!m_hasValue)
      {
        // throwing here prohibited by our guidelines
        // https://azure.github.io/azure-sdk/cpp_design.html#pre-conditions
        std::abort();
      }

      return m_value;
    }

    T&& GetValue() && noexcept
    {
      if (!m_hasValue)
      {
        // throwing here prohibited by our guidelines
        // https://azure.github.io/azure-sdk/cpp_design.html#pre-conditions
        std::abort();
      }

      return std::move(m_value);
    }

    // GetValueOrDefault() does NOT make sense here given that T
    // in C++ isn't guaranteed a default.
  };
}} // namespace Azure::Core
