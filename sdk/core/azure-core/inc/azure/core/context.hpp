// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Context for canceling long running operations.
 */

#pragma once

#include "azure/core/datetime.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <new> //For the non-allocating placement new
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Azure { namespace Core {

  /**
   * @brief An exception that gets thrown when some operation is cancelled.
   *
   */
  class OperationCancelledException : public std::runtime_error {
  public:
    /**
     * @brief Construct with message string as description.
     *
     * @param message The description for the exception.
     */
    explicit OperationCancelledException(std::string const& message) : std::runtime_error(message)
    {
    }
  };

  /**
   * @brief A context is a node within a tree that represents expiration times and key/value pairs.
   */
  class Context {
  private:
    struct ContextSharedState
    {
      std::shared_ptr<ContextSharedState> Parent;
      std::atomic<DateTime::rep> Expiration;
      std::string Key;
      std::shared_ptr<void> Value;
      const std::type_info& ValueType;

      static constexpr DateTime::rep ToDateTimeRepresentation(DateTime const& dateTime)
      {
        return dateTime.time_since_epoch().count();
      }

      static constexpr DateTime FromDateTimeRepresentation(DateTime::rep dtRepresentation)
      {
        return DateTime(DateTime::time_point(DateTime::duration(dtRepresentation)));
      }

      explicit ContextSharedState()
          : Expiration(ToDateTimeRepresentation((DateTime::max)())), Value(nullptr),
            ValueType(typeid(std::nullptr_t))
      {
      }

      explicit ContextSharedState(
          const std::shared_ptr<ContextSharedState>& parent,
          DateTime const& expiration)
          : Parent(parent), Expiration(ToDateTimeRepresentation(expiration)), Value(nullptr),
            ValueType(typeid(std::nullptr_t))
      {
      }

      template <class T>
      explicit ContextSharedState(
          const std::shared_ptr<ContextSharedState>& parent,
          DateTime const& expiration,
          const std::string& key,
          T value) // NOTE, should this be T&&
          : Parent(parent), Expiration(ToDateTimeRepresentation(expiration)), Key(key),
            Value(std::make_shared<T>(std::move(value))), ValueType(typeid(T))
      {
      }
    };

    std::shared_ptr<ContextSharedState> m_contextSharedState;

    explicit Context(std::shared_ptr<ContextSharedState> impl)
        : m_contextSharedState(std::move(impl))
    {
    }

  public:
    /**
     * @brief Construct a new context with no expiration, and no value associated.
     */
    Context() : m_contextSharedState(std::make_shared<ContextSharedState>()) {}

    /**
     * @brief Copy constructor.
     */
    Context& operator=(const Context&) = default;

    /**
     * @brief Create a context with expiration.
     *
     * @param expiration A point in time after which a context expires.
     *
     * @return A child context with expiration.
     */
    Context CreateWithExpiration(DateTime const& expiration) const
    {
      return Context{std::make_shared<ContextSharedState>(m_contextSharedState, expiration)};
    }

    /**
     * @brief Create a context without an expiration, but with \p key and \p value associated with
     * it.
     *
     * @param key A key to associate with this context.
     * @param value A value to associate with this context.
     *
     * @return A child context with no expiration and the \p key and \p value associated with it.
     */
    template <class T> Context CreateWithValue(const std::string& key, T&& value) const
    {
      return Context{std::make_shared<ContextSharedState>(
          m_contextSharedState, (DateTime::max)(), key, std::forward<T>(value))};
    }

    /**
     * @brief Get an expiration time point associated with this context or the branch of contexts
     * this context belongs to.
     *
     * @return An expiration associated with the context found; an empty value if a specific value
     * can't be found.
     */
    DateTime GetExpiration() const;

    /**
     * @brief Get a value associated with a \p key parameter within this context or the branch of
     * contexts this context belongs to.
     *
     * @param key A key associated with a context to find.
     *
     * @return A value associated with the context found; an empty value if a specific value can't
     * be found.
     */
    template <class T> const T& GetValue(const std::string& key) const
    {
      if (!key.empty())
      {
        for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
        {
          if (ptr->Key == key)
          {
            if (typeid(T) != ptr->ValueType)
            {
              // type mismatch
              std::abort();
            }
            return *reinterpret_cast<const T*>(ptr->Value.get());
          }
        }
      }
      std::abort();
      // It should be expected that keys may not exist
      //  That implies we return T* and NOT a T&
    }

    /**
     * @brief Check whether the context has a key matching \p key parameter in it itself, or in the
     * branch the context belongs to.
     *
     * @param key A key associated with a context to find.
     *
     * @return `true` if this context, or the tree branch this context belongs to has a \p key
     * associated with it. `false` otherwise.
     */
    bool HasKey(const std::string& key) const
    {
      if (!key.empty())
      {
        for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
        {
          if (ptr->Key == key)
          {
            return true;
          }
        }
      }
      return false;
    }

    /**
     * @brief Cancels the context.
     */
    void Cancel()
    {
      m_contextSharedState->Expiration
          = ContextSharedState::ToDateTimeRepresentation((DateTime::min)());
    }

    /**
     * @brief Check if the context is cancelled.
     * @return `true` if this context is cancelled, `false` otherwise.
     */
    bool IsCancelled() const { return GetExpiration() < std::chrono::system_clock::now(); }

    /**
     * @brief Throw an exception if the context was cancelled.
     */
    void ThrowIfCancelled() const
    {
      if (IsCancelled())
      {
        throw OperationCancelledException("Request was cancelled by context.");
      }
    }

    /**
     * @brief Get the application context (root).
     */
    static Context& GetApplicationContext();
  };
}} // namespace Azure::Core
