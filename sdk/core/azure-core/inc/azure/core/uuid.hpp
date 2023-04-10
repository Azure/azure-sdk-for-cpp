// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Universally unique identifier.
 */

#pragma once

#include "azure/core/platform.hpp"

#include <array>
#include <cstring>
#include <string>

namespace Azure { namespace Core {
  /**
   * @brief Universally unique identifier.
   */
  class Uuid final {

  private:
    static constexpr size_t UuidSize = 16;

    std::array<uint8_t, UuidSize> m_uuid;
    // The UUID reserved variants.
    static constexpr uint8_t ReservedNCS = 0x80;
    static constexpr uint8_t ReservedRFC4122 = 0x40;
    static constexpr uint8_t ReservedMicrosoft = 0x20;
    static constexpr uint8_t ReservedFuture = 0x00;

  private:
    Uuid(uint8_t const uuid[UuidSize]) { std::memcpy(m_uuid.data(), uuid, UuidSize); }

  public:
    /**
     * @brief Gets Uuid as a string.
     * @details A string is in canonical format (4-2-2-2-6 lowercase hex and dashes only).
     */
    std::string ToString();

    /**
     * @brief Construct a Uuid from an existing UUID represented as an array of bytes.
     * @details Creates a Uuid from a UUID created in an external scope.
     */
    Uuid(std::array<uint8_t, UuidSize> const& uuid) { m_uuid = uuid; }

    /**
     * @brief Returns the internal value of the Uuid for consumption by clients who need non-string
     * representation of the Uuid
     * @returns An array with the binary representation of the Uuid.
     */
    operator std::array<uint8_t, UuidSize> const&() const { return m_uuid; }

    /**
     * @brief Creates a new random UUID.
     *
     */
    static Uuid CreateUuid();
  };
}} // namespace Azure::Core
