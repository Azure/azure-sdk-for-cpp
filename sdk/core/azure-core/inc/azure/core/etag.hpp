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
   * @brief Represents an HTTP ETag.
   */
  class ETag {
  private:
    Nullable<std::string> m_value;

  private:
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
    //

    */
    bool ETagComparison(const ETag& left, const ETag& right) const
    {

      // If both values are empty then we consider the ETag equal
      if (!left.m_value && !right.m_value)
        return true;

      if (!left.m_value || !right.m_value)
        return false;

      // Previous comparison checked that both are Weak or both are strong
      //  Thuse we can decide which comparison to do based on one of them
      if (!left.IsWeak())
      {
        // Strong ETag so comparison is that the values match character for character
        return left.m_value.GetValue().compare(right.m_value.GetValue()) == 0;
      }
      else
      {
        // Left side is weak ETag
        if (right.IsWeak())
        {
          return left.m_value.GetValue().compare(right.m_value.GetValue()) == 0;
        }
        // Trim the weak tag
        auto leftLength = left.m_value.GetValue().length();

        if (leftLength - 2 != right.m_value.GetValue().length())
        {
          return false;
        }
        return left.m_value.GetValue().compare(2, leftLength - 2, right.m_value.GetValue()) == 0;
      }
      return false;
    }

  protected:
    /**
     * @brief Construct an empty (null) #ETag.
     */
    ETag() {}

  public:
    /**
     * @brief Construct a #ETag.
     */
    explicit ETag(std::string etag) : m_value(std::move(etag)) {}

    /**
     * @brief Copy assignment.
     */
    ETag& operator=(const ETag&) = default;

    /**
     * @brief Whether @Tag is present.
     * @return `true` if @ETag has a value, `false` otherwise.
     */
    const bool HasValue() { return m_value.HasValue(); }

    /*
     * @brief Return the contained string value
     * @return #std::string
     */
    const std::string& ToString()
    {
      if (!m_value.HasValue())
      {
        abort();
      }
      return m_value.GetValue();
    }

    /**
     * @brief Compare with \p other @ETag for equality.
     * @param other Other @ETag to compare with.
     * @return `true` if @ETag instances are equal, `false` otherwise.
     */
    bool operator==(const ETag& other) const { return ETagComparison(*this, other); }

    /**
     * @brief Compare with \p other @ETag for inequality.
     * @param other Other @ETag to compare with.
     * @return `true` if @ETag instances are not equal, `false` otherwise.
     */
    bool operator!=(const ETag& other) const { return !(*this == other); }

    /**
     * @brief Compare with \p other @ETag for inequality.
     * @param other Other @ETag to compare with.
     * @return `true` if @ETag instances are not equal, `false` otherwise.
     */
    bool IsWeak() const
    {
      // A null eTag is considered strong
      if (!m_value)
      {
        return false;
      }

      auto& val = m_value.GetValue();
      // Shortest valid weak etag is 4
      //  W/""
      if (val.length() < 4)
      {
        return false;
      }

      // Valid format is W/""
      //   Must end with a " in the string
      if (val[0] == 'W' && val[1] == '/' && val[2] == '"' && val[val.size() - 1] == '"')
      {
        return true;
      }

      return false;
    }

    /**
     * @brief @ETag representing everything
     * @notes The any ETag is *, (unquoted).  It is NOT the same as "*"
     */
    static const ETag& Any()
    {
      static ETag Any = ETag("*");
      return Any;
    }

    /**
     * @brief @ETag representing no @ETag present
     */
    static const ETag& Null()
    {
      static ETag Null = ETag();
      return Null;
    }
  };
}} // namespace Azure::Core
