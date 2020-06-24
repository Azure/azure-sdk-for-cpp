// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib> // for abort
#include <new> // for placement new
#include <type_traits>
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
      }
    }

    ~Nullable()
    {
      if (m_hasValue)
      {
        m_value.~T();
      }
    }

    void Reset() noexcept /* enforces termination */ { Destroy(); }

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

    // Intentionally lowercase to override the swap
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

    template <
        class U = T,
        typename std::enable_if<
            !std::is_same<Nullable, std::remove_cv_t<std::remove_reference_t<U>>>::value // Avoid repeated assignment
            && !(std::is_scalar<U>::value && std::is_same<T, std::decay_t<U>>::value) // Avoid repeated assignment of equivallent scaler types
            && std::is_constructible<T, U>::value // Ensure the type is constructible
            && std::is_assignable<T, U>::value // Ensure the type is assignable
            && !std::is_null_pointer<U>::value, // Dissallow nullptr assignment
            int>::type
        = 0>
    Nullable& operator=(Nullable&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
      // this move and swap may be inefficient for some Ts but
      // it's a lot less code than the standard implementation :)
      Nullable{std::move(other)}.Swap(*this);
      return *this;
    }

    template <
        class U = T,
        typename std::enable_if<
            !std::is_same<Nullable, std::remove_cv_t<std::remove_reference_t<U>>>::value // Avoid repeated assignment
            && !(std::is_scalar<U>::value && std::is_same<T, std::decay_t<U>>::value) //Avoid repeated assignment of equivallent scaler types
            && std::is_constructible<T, U>::value   //Ensure the type is constructible
            && std::is_assignable<T, U>::value      //Ensure the type is assignable
            && !std::is_null_pointer<U>::value,     //Dissallow nullptr assignment
            int>::type
        = 0>
    Nullable& operator=(const U& other)
    {
      // this copy and swap may be inefficient for some Ts but
      // it's a lot less code than the standard implementation :)
      Nullable{other}.Swap(*this);
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

    explicit operator bool() const { return HasValue(); }

    // GetValueOrDefault() does NOT make sense here given that T
    // in C++ isn't guaranteed a default.
  };
}} // namespace Azure::Core
