// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define ETag.
 */

#pragma once

#include "azure/core/nullable.hpp"

#include <string>

namespace Azure { namespace Core {

  /**
   * @brief Represents an HTTP validator.
   */
  class ETag {
    // ETag is a validator based on https://tools.ietf.org/html/rfc7232#section-2.3.2
  private:
    Nullable<std::string> m_value;

  public:
    /**
     * @brief The comparison type.
     */
    enum class ETagComparison
    {
      Strong,
      Weak
    };

    /*
    2.3.2.  Comparison

      There are two entity-tag comparison functions, depending on whether
      or not the comparison context allows the use of weak validators:

      o  Strong comparison: two entity-tags are equivalent if both are not
          weak and their opaque-tags match character-by-character.

      o  Weak comparison: two entity-tags are equivalent if their
          opaque-tags match character-by-character, regardless of either or
          both being tagged as "weak".

       +--------+--------+-------------------+-----------------+
       | ETag 1 | ETag 2 | Strong Comparison | Weak Comparison |
       +--------+--------+-------------------+-----------------+
       | W/"1"  | W/"1"  | no match          | match           |
       | W/"1"  | W/"2"  | no match          | no match        |
       | W/"1"  | "1"    | no match          | match           |
       | "1"    | "1"    | match             | match           |
       +--------+--------+-------------------+-----------------+

    // etag:                            //This is possible and means no etag is present
    // etag:""
    // etag:"*"                         //This means the etag is value '*'
    // etag:"some value"                //This means the etag is value 'some value'
    // etag:/W""                        //Weak eTag
    // etag:*                           //This is special, means any etag
    // If-Match header can do this
    // If-Match:"value1","value2","value3"  // Do this if any of these match

    */

    /*
     * @brief Indicates whether two #Azure::Core::ETag values are equal.
     * @param left #Azure::Core::ETag to compare.
     * @param right #Azure::Core::ETag to compare.
     * @param comparisonKind Determines what #Azure::Core::ETag::ETagComparison to perform, default
     * is #Azure::Core::ETag::ETagComparison Strong.
     * @return `true` if #Azure::Core::ETag matches, `false` otherwise.
     */
    static bool Equals(
        const ETag& left,
        const ETag& right,
        const ETagComparison comparisonKind = ETagComparison::Strong)
    {
      // ETags are != if one of the values is null
      if (!left.m_value || !right.m_value)
      {
        // Caveat, If both values are null then we consider the ETag equal
        return !left.m_value && !right.m_value;
      }

      switch (comparisonKind)
      {
        case ETagComparison::Strong:
          // Strong comparison
          // If either is weak then there is no match
          //  else tags must match character for character
          return !left.IsWeak() && !right.IsWeak()
              && (left.m_value.GetValue().compare(right.m_value.GetValue()) == 0);
          break;

        case ETagComparison::Weak:

          auto leftStart = left.IsWeak() ? 2 : 0;
          auto rightStart = right.IsWeak() ? 2 : 0;

          auto leftVal = left.m_value.GetValue();
          auto rightVal = right.m_value.GetValue();

          // Compare if lengths are equal
          //   Compare the strings character by character
          return ((leftVal.length() - leftStart) == (rightVal.length() - rightStart))
              && (leftVal.compare(leftStart, leftVal.length() - leftStart, &rightVal[rightStart])
                  == 0);
          break;
      }
      // Unknown comparison
      abort();
    }

    /**
     * @brief Construct an empty (null) #Azure::Core::ETag.
     */
    ETag() = default;

    /**
     * @brief Construct a #Azure::Core::ETag.
     * @param etag The string value representation.
     */
    explicit ETag(std::string etag) : m_value(std::move(etag)) {}

    /**
     * @brief Whether #Azure::Core::ETag is present.
     * @return `true` if #Azure::Core::ETag has a value, `false` otherwise.
     */
    bool HasValue() const { return m_value.HasValue(); }

    /*
     * @brief Returns the resource metadata represented as a string.
     * @return #std::string
     */
    const std::string& ToString() const
    {
      if (!m_value.HasValue())
      {
        abort();
      }
      return m_value.GetValue();
    }

    /**
     * @brief Compare with \p other #Azure::Core::ETag for equality.
     * @param other Other #Azure::Core::ETag to compare with.
     * @return `true` if #Azure::Core::ETag instances are equal according to strong validation,
     * `false` otherwise.
     */
    bool operator==(const ETag& other) const
    {
      return Equals(*this, other, ETagComparison::Strong);
    }

    /**
     * @brief Compare with \p other #Azure::Core::ETag for inequality.
     * @param other Other #Azure::Core::ETag to compare with.
     * @return `true` if #Azure::Core::ETag instances are not equal according to strong validation,
     * `false` otherwise.
     */
    bool operator!=(const ETag& other) const { return !(*this == other); }

    /**
     * @brief Specifies whether the #Azure::Core::ETag is strong or weak.
     * @return `true` if #Azure::Core::ETag is a weak validator, `false` otherwise.
     */
    bool IsWeak() const
    {
      // Null ETag is considered Strong
      // Shortest valid weak etag has length of 4
      //  W/""
      // Valid weak format must start with W/"
      //   Must end with a /"
      const bool weak = m_value && (m_value.GetValue().length() >= 4)
          && ((m_value.GetValue()[0] == 'W') && (m_value.GetValue()[1] == '/')
              && (m_value.GetValue()[2] == '"')
              && (m_value.GetValue()[m_value.GetValue().size() - 1] == '"'));

      return weak;
    }

    /**
     * @brief #Azure::Core::ETag representing everything.
     * @note The any #Azure::Core::ETag is *, (unquoted).  It is NOT the same as "*".
     */
    static const ETag& Any()
    {
      static ETag any = ETag("*");
      return any;
    }
  };
}} // namespace Azure::Core
