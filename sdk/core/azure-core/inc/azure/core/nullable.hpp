// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Manages an optional contained value, i.e. a value that may or may not be present.
 */

#pragma once

#include <cstdlib> // for abort
#include <new> // for placement new
#include <type_traits>
#include <utility> // for swap and move

namespace Azure { namespace Core {
  namespace Details {
    struct NontrivialEmptyType
    {
      constexpr NontrivialEmptyType() noexcept {}
    };
  } // namespace Details

  /**
   * @brief Manages an optional contained value, i.e. a value that may or may not be present.
   *
   * @tparam T A type to represent contained values.
   */
  template <class T> class Nullable {
    union
    {
      Details::NontrivialEmptyType
          m_disengaged; // due to constexpr rules for the default constructor
      T m_value;
    };

    bool m_hasValue;

  public:
    /**
     * @brief Construct a #Azure::Core::Nullable that represents the absence of value.
     */
    constexpr Nullable() : m_disengaged{}, m_hasValue(false) {}

    /**
     * @brief Construct a #Azure::Core::Nullable having an \p initialValue.
     *
     * @param initialValue A non-absent value to initialize with.
     */
    constexpr Nullable(T initialValue) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_value(std::move(initialValue)), m_hasValue(true)
    {
    }

    /// Copy constructor.
    Nullable(const Nullable& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
        : m_hasValue(other.m_hasValue)
    {
      if (m_hasValue)
      {
        ::new (static_cast<void*>(&m_value)) T(other.m_value);
      }
    }

    /// Move constructor.
    Nullable(Nullable&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_hasValue(other.m_hasValue)
    {
      if (m_hasValue)
      {
        ::new (static_cast<void*>(&m_value)) T(std::move(other.m_value));
      }
    }

    /**
     * @brief Destroy the contained value, if there is one.
     */
    ~Nullable()
    {
      if (m_hasValue)
      {
        m_value.~T();
      }
    }

    /**
     * @brief Destroy any contained value, if there is one.
     */
    void Reset() noexcept /* enforces termination */
    {
      if (m_hasValue)
      {
        m_value.~T();
        m_hasValue = false;
      }
    }

    /**
     * @brief Exchange the contents.
     *
     * @param other An instance to exchange the contents with.
     */
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
          Reset();
        }
      }
      else if (other.m_hasValue)
      {
        ::new (static_cast<void*>(&m_value)) T(std::move(other.m_value)); // throws
        m_hasValue = true;
        other.Reset();
      }
    }

    /**
     * @brief Invokes #Azure::Core::Nullable::Swap while having a lowercase name that satisfies
     * `swappable` requirements (see details).
     *
     * @details Swappable requirements: https://en.cppreference.com/w/cpp/named_req/Swappable
     */
    friend void swap(Nullable& lhs, Nullable& rhs) noexcept(
        std::is_nothrow_move_constructible<T>::value)
    {
      lhs.Swap(rhs);
    }

    /// Assignment operator.
    Nullable& operator=(const Nullable& other)
    {
      // this copy and swap may be inefficient for some Ts but
      // it's a lot less code than the standard implementation :)
      Nullable{other}.Swap(*this);
      return *this;
    }

    /// Assignment operator with move semantics.
    Nullable& operator=(Nullable&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
      // this move and swap may be inefficient for some Ts but
      // it's a lot less code than the standard implementation :)
      Nullable{std::move(other)}.Swap(*this);
      return *this;
    }

    /**
     * @brief Assignment operator from another type.
     *
     * @tparam U Type of \p other.
     *
     * @param other Other #Azure::Core::Nullable.
     */
    template <
        class U = T,
        typename std::enable_if<
            !std::is_same<
                Nullable,
                typename std::remove_cv<typename std::remove_reference<U>::type>::type>::
                    value // Avoid repeated assignment
                && !(
                    std::is_scalar<U>::value
                    && std::is_same<T, typename std::decay<U>::type>::value) // Avoid repeated
                                                                             // assignment of
                                                                             // equivalent scalar
                                                                             // types
                && std::is_constructible<T, U>::value // Ensure the type is constructible
                && std::is_assignable<T&, U>::value, // Ensure the type is assignable
            int>::type
        = 0>
    Nullable& operator=(U&& other) noexcept(
        std::is_nothrow_constructible<T, U>::value&& std::is_nothrow_assignable<T&, U>::value)
    {
      if (m_hasValue)
      {
        m_value = std::forward<U>(other);
      }
      else
      {
        ::new (static_cast<void*>(&m_value)) T(std::forward<U>(other));
        m_hasValue = true;
      }
      return *this;
    }

    /**
     * @brief Construct the contained value in-place.
     *
     * @details If this instance already contains a value before the call, the contained value is
     * destroyed by calling its destructor.
     */
    template <class... U>
    T& Emplace(U&&... Args) noexcept(std::is_nothrow_constructible<T, U...>::value)
    {
      Reset();
      ::new (static_cast<void*>(&m_value)) T(std::forward<U>(Args)...);
      return m_value;
    }

    /**
     * @brief Check whether a value is contained.
     *
     * @return `true` If a value is contained, `false` if value is absent.
     */
    bool HasValue() const noexcept { return m_hasValue; }

    /**
     * @brief Get the contained value.
     */
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

    /**
     * @brief Get the contained value reference.
     */
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

    /**
     * @brief Get the contained value (as rvalue reference).
     */
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

    /**
     * @brief `operator bool` on the condition of #Azure::Core::Nullable::HasValue.
     */
    explicit operator bool() const noexcept { return HasValue(); }

    /**
     * @brief Get the contained value, returns \p other if value is absent.
     * @param other A value to return when no value is contained.
     * @return A contained value (when present), or \p other.
     */
    template <
        class U = T,
        typename std::enable_if<
            std::is_convertible<const T&, typename std::remove_cv<T>::type>::value
                && std::is_convertible<U, T>::value,
            int>::type
        = 0>
    constexpr typename std::remove_cv<T>::type ValueOr(U&& other) const&
    {
      if (m_hasValue)
      {
        return m_value;
      }

      return static_cast<typename std::remove_cv<T>::type>(std::forward<U>(other));
    }

    /**
     * @brief Get the contained value, returns \p other if value is absent.
     * @param other A value to return when no value is contained.
     * @return A contained value (when present), or \p other.
     */
    template <
        class U = T,
        typename std::enable_if<
            std::is_convertible<T, typename std::remove_cv<T>::type>::value
                && std::is_convertible<U, T>::value,
            int>::type
        = 0>
    constexpr typename std::remove_cv<T>::type ValueOr(U&& other) &&
    {
      if (m_hasValue)
      {
        return std::move(m_value);
      }

      return static_cast<typename std::remove_cv<T>::type>(std::forward<U>(other));
    }
  };
}} // namespace Azure::Core
