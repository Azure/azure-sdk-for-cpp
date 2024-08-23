// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Universally unique identifier.
 */

#pragma once

#include "azure/core/platform.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace Azure { namespace Core {
  /**
   * @brief Universally unique identifier.
   */
  class Uuid final {
  public:
    using ValueArray = std::array<std::uint8_t, 16>;

  private:
    ValueArray m_uuid{};

  private:
    constexpr Uuid(ValueArray const& uuid) : m_uuid(uuid) {}

  public:
    /**
     * @brief Constructs a Nil UUID (`00000000-0000-0000-0000-000000000000`).
     *
     */
    constexpr explicit Uuid() : m_uuid{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} {}

    /**
     * @brief Gets Uuid as a string.
     * @details A string is in canonical format (`8-4-4-4-12` lowercase hex and dashes only).
     */
    std::string ToString() const;

    /**
     * @brief Returns the binary value of the Uuid for consumption by clients who need non-string
     * representation of the Uuid.
     * @returns An array with the binary representation of the Uuid.
     */
    constexpr ValueArray const& AsArray() const { return m_uuid; }

    /**
     * @brief Creates a new random UUID.
     *
     */
    static Uuid CreateUuid();

    /**
     * @brief Construct a Uuid from an existing UUID represented as an array of bytes.
     * @details Creates a Uuid from a UUID created in an external scope.
     */
    static constexpr Uuid CreateFromArray(ValueArray const& uuid) { return Uuid{uuid}; }

    /**
     * @brief Construct a Uuid by parsing its representation.
     * @param s a string in `8-4-4-4-12` hex characters format.
     * @throw `std::invalid_argument` if \p s cannot be parsed.
     */
    static Uuid Parse(std::string const& s);

    constexpr bool operator==(Uuid const& other) const
    {
      // std::array::operator==() is not a constexpr until C++20
      for (size_t i = 0; i < m_uuid.size(); ++i)
      {
        if (m_uuid[i] != other.m_uuid[i])
        {
          return false;
        }
      }

      return true;
    }

    constexpr bool operator!=(Uuid const& other) const { return !(*this == other); }

    /**
     * @brief Checks if the value represents a Nil UUID (`00000000-0000-0000-0000-000000000000`).
     *
     */
    constexpr bool IsNil() const
    {
      for (size_t i = 0; i < m_uuid.size(); ++i)
      {
        if (m_uuid[i] != 0)
        {
          return false;
        }
      }

      return true;
    }
  };
}} // namespace Azure::Core
