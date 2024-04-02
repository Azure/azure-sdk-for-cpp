// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file enum_operators.hpp
 * @brief Defines bitwise operators for enums.
 * @details This file defines bitwise operators for enum classes. This allows the use of the
 * operators |, |=, &, &=, ^, ^=, and ~ with enum classes. This is useful for flags enums.
 * Example: enum class MyEnum { A = 1, B = 2, C = 4 }; MyEnum e = MyEnum::A | MyEnum::B;
 * Example: enum class MyEnum { A = 1, B = 2, C = 4 }; MyEnum e = MyEnum::A; e &= MyEnum::B;
 */
#pragma once

#include <type_traits>

namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Bitwise OR operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>>
  constexpr E operator|(E lhs, E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  /**
   * @brief Bitwise OR EQUALS operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>>
  constexpr E& operator|=(E& lhs, E rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  /**
   * @brief Bitwise AND operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>>
  constexpr E operator&(E lhs, E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief Bitwise AND EQUALS operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>>
  constexpr E& operator&=(E& lhs, E rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  /**
   * @brief Bitwise XOR operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>>
  constexpr E operator^(E lhs, E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<type>(lhs) ^ static_cast<type>(rhs));
  }

  /**
   * @brief Bitwise XOR EQUALS operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>>
  constexpr E& operator^=(E& lhs, E rhs)
  {
    lhs = lhs ^ rhs;
    return lhs;
  }

  /**
   * @brief Bitwise COMPLEMENT operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>> constexpr E operator~(E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<type>(rhs));
  }
}}} // namespace Azure::Data::Tables
