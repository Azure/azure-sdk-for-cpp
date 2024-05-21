// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Manages an optional contained value, i.e. a value that may or may not be present.
 */

#pragma once

#include "azure/core/azure_assert.hpp"

#include <memory>
#include <new> // for placement new
#include <type_traits>
#include <utility> // for swap and move

namespace Azure {
namespace _detail {
  struct NontrivialEmptyType final
  {
    constexpr NontrivialEmptyType() noexcept {}
  };
} // namespace _detail

/**
 * @brief Manages an optional contained value, i.e. a value that may or may not be present.
 *
 * @tparam T A type to represent contained values.
 */
template <class T> class Nullable final {
  union
  {
    _detail::NontrivialEmptyType m_disengaged; // due to constexpr rules for the default constructor
    T m_value;
  };

  bool m_hasValue;

public:
  /**
   * @brief Constructs a `%Nullable` that represents the absence of value.
   *
   */
  constexpr Nullable() : m_disengaged{}, m_hasValue(false) {}

  /**
   * @brief Constructs a `%Nullable` having an \p initialValue.
   *
   * @param initialValue A non-absent value to initialize with.
   */
  constexpr Nullable(T initialValue) noexcept(std::is_nothrow_move_constructible<T>::value)
      : m_value(std::move(initialValue)), m_hasValue(true)
  {
  }

  /**
   * @brief Constructs a `%Nullable` by copying another `%Nullable`.
   *
   * @param other Another `%Nullable` instance to copy.
   */
  Nullable(const Nullable& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
      : m_disengaged{}, m_hasValue(other.m_hasValue)
  {
    if (m_hasValue)
    {
      ::new (static_cast<void*>(&m_value)) T(other.m_value);
    }
  }

  /**
   * @brief Constructs a `%Nullable` by moving in another `%Nullable`.
   *
   * @param other A `%Nullable` instance to move into the instance being constructed.
   */
  Nullable(Nullable&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
      : m_disengaged{}, m_hasValue(other.m_hasValue)
  {
    if (m_hasValue)
    {
      ::new (static_cast<void*>(&m_value)) T(std::move(other.m_value));
    }
  }

  /**
   * @brief Destructs the `%Nullable`, calling the destructor for the contained value if there is
   * one.
   *
   */
  ~Nullable()
  {
    if (m_hasValue)
    {
      m_value.~T();
    }
  }

  /**
   * @brief Destructs the contained value, if there is one.
   *
   */
  void Reset() noexcept(std::is_nothrow_destructible<T>::value) /* enforces termination */
  {
    if (m_hasValue)
    {
      m_hasValue = false;
      m_value.~T();
    }
  }

  /**
   * @brief Exchanges the contents.
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
   * @brief Invokes #Azure::Nullable::Swap while having a lowercase name that satisfies
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

  template <typename T> class Nullable<T&> final {
    std::unique_ptr<T> m_value;

  public:
    constexpr Nullable() {}

    Nullable(T initialValue) : m_value(std::make_unique<T>(std::move(initialValue))) {}

    Nullable(const Nullable& other)
    {
      if (other.m_value)
      {
        m_value = std::make_unique<T>(*other.m_value);
      }
    }

    // Should we also have an overload for const Nullable<T>&, and add the same for non-specialized
    // version, so that Nullable<T>s and Nullable<T&>s can be assigned to each other?

    Nullable(Nullable&& other) noexcept : m_value(std::move(other.m_value)) {}

    Nullable& operator=(Nullable const& other) if (other.m_value)
    {
      m_value = std::make_unique<T>(*other.m_value);
    }

    Nullable& operator=(Nullable&& other) noexcept { m_value = std::move(other.value); }

    void Reset() { m_value.reset(); }

    void Swap(Nullable& other) noexcept { m_value.swap(other.m_value); }

    friend void swap(Nullable& lhs, Nullable& rhs) noexcept { lhs.Swap(rhs); }

    // TODO: Write assignment operator from another type (Nullable& operator=(U&& other)),
    // same as non-specialized version has. I'm not in the mood to detail it right now.


    // TODO: implement similar to the non-specialized version
    // template <class... U>
    // T& Emplace(U&&... Args)

    bool HasValue() const noexcept { return m_value; }

    const T& Value() const& noexcept
    {
      AZURE_ASSERT_MSG(m_value, "Empty Nullable, check HasValue() first.");
      return *m_value;
    }

    T& Value() & noexcept
    {
      AZURE_ASSERT_MSG(m_value, "Empty Nullable, check HasValue() first.");

      return *m_value;
    }

    // TODO: T&& Value() && noexcept

    explicit operator bool() const noexcept { return m_value; }

    // Somehow, it is only now that I found out that NUllable has these operators, pretty cool.
    const T* operator->() const { return m_value.operator->(); }
    T* operator->() { return m_value.operator->(); }

    const T& operator*() const& { return *m_value; }
    T& operator*() & { return *m_value; }

    // TODO: constexpr T&& operator*() && { return std::move(m_value); }
    // and constexpr const T&& operator*() const&& { return std::move(m_value); }

    // TODO: ValueOr(U&& other) const&
    // and ValueOr(U&& other) &&

    // Temporary, to make sure that it is actually this specialization that gets instantiated.
    // This absolutely should get deleted in the final version!
    // For the tets, there are better ways to ensure.
    constexpr bool IsTemplateSpecialization() const { return true; }
  };
} // namespace Azure
