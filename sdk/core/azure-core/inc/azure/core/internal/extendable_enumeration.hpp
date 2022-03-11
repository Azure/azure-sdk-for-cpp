// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal utility functions for extendable enumerations.
 *
 */
#include <string>

namespace Azure { namespace Core { namespace _internal {
  /** @brief Template base class helper for implementing extendable enumerations.
   *
   * This template exists to simplify the experience of authoring ["extendable
   * enumerations"](https://azure.github.io/azure-sdk/cpp_implementation.html#cpp-enums).
   *
   * An extendable enumeration derives from the #ExtendableEnumeration base class passing in the
   * extendable enumeration type as the template specialization.
   *
   * Example:
   *
   * \code{.cpp}
   * class MyEnumeration final : public ExtendableEnumeration<AttestationType> {
   * public:
   *   MyEnumeration(std::string attestationType) :
   *     ExtendableEnumeration(std::move(attestationType)) {}
   *   MyEnumeration() = default;
   *   static const MyEnumeration Enumerator1;
   *   static const MyEnumeration Enumerator2;
   *   static const MyEnumeration Enumerator3;
   * };
   * \endcode
   *
   */
  template <class T> class ExtendableEnumeration {
  private:
    std::string m_enumerationValue;

  protected:
    ~ExtendableEnumeration() = default;

  public:
    /**
     * @brief Construct a new extensable enumeration object
     *
     * @param enumerationValue The string enumerationValue used for the value.
     */
    explicit ExtendableEnumeration(std::string enumerationValue)
        : m_enumerationValue(std::move(enumerationValue))
    {
    }

    /**
     * @brief Construct a default extendable enumeration.
     */
    ExtendableEnumeration() = default;

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another extendable enumeration to be compared.
     */
    bool operator==(T const& other) const noexcept
    {
      return m_enumerationValue == other.m_enumerationValue;
    }

    /**
     * @brief Enable comparing the ext enum.
     *
     * @param other Another extendable enumeration to be compared.
     */
    bool operator!=(T const& other) const noexcept { return !operator==(other); }

    /**
     * @brief Return the ExtendableEnumeration string representation.
     *
     */
    std::string const& ToString() const { return m_enumerationValue; }
  };
}}} // namespace Azure::Core::_internal
