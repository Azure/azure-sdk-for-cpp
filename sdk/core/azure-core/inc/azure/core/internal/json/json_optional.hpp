// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define a convenience layer on top of Json for setting optional fields.
 *
 */

#pragma once

#include "azure/core/internal/json/json.hpp"
#include "azure/core/nullable.hpp"

#include <functional>
#include <string>

namespace Azure { namespace Core { namespace Json { namespace _internal {

  /**
   * @brief Define a wrapper for working with Json containing optional fields.
   *
   */
  struct JsonOptional
  {
    /**
     * @brief If the optional key \p key is present in the json node \p jsonKey set the value of \p
     * destination.
     *
     * @remark If the key is not in the json node, the \p destination is not modified.
     *
     * @param jsonKey The json node to review.
     * @param key The key name for the optional property.
     * @param destination The value to update if the key name property is in the json node.
     */
    template <class T>
    static inline void SetIfExists(
        Azure::Nullable<T>& destination,
        Azure::Core::Json::_internal::json const& jsonKey,
        std::string const& key) noexcept
    {
      if (jsonKey.contains(key))
      {
        destination = jsonKey[key].get<T>();
      }
    }

    /**
     * @brief If the optional key \p key is present in the json node \p jsonKey set the value of \p
     * destination.
     *
     * @remark If the key is not in the json node, the \p destination is not modified.
     *
     * @param jsonKey The json node to review.
     * @param key The key name for the optional property.
     * @param destination The value to update if the key name property is in the json node.
     * @param decorator A optional function to update the json value before updating the \p
     * destination.
     */
    template <class T, class V>
    static inline void SetIfExists(
        Azure::Nullable<V>& destination,
        Azure::Core::Json::_internal::json const& jsonKey,
        std::string const& key,
        std::function<V(T value)> decorator) noexcept
    {
      if (jsonKey.contains(key))
      {
        destination = decorator(jsonKey[key].get<T>());
      }
    }
  };

}}}} // namespace Azure::Core::Json::_internal
