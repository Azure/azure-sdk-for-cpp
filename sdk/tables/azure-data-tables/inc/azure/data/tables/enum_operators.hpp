// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <type_traits>

namespace Azure { namespace Data { namespace Tables {
  /**
   * @brief Bitwise OR operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>> E operator|(E lhs, E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<type>(lhs) | static_cast<type>(rhs));
  }

  /**
   * @brief Bitwise AND operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>> E operator&(E lhs, E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<type>(lhs) & static_cast<type>(rhs));
  }

  /**
   * @brief Bitwise NOT operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>> E operator!(E val)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(!static_cast<type>(val));
  }

  /**
   * @brief Bitwise XOR operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>> E operator^(E lhs, E rhs)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<type>(lhs) ^ static_cast<type>(rhs));
  }

  /**
   * @brief Bitwise COMPLEMENT assignment operator for enum class.
   */
  template <class E, class = std::enable_if_t<std::is_enum<E>{}>> E operator~(E val)
  {
    using type = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<type>(val));
  }
}}} // namespace Azure::Data::Tables
