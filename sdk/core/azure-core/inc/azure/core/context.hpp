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
   * @brief A context is a node within a tree that represents deadlines and key/value pairs.
   */
  class Context {
  public:
    /**
     * @brief A context key.
     */
    class Key final {
      Key const* m_uniqueAddress;

    public:
      Key() : m_uniqueAddress(this) {}

      bool operator==(Key const& other) const
      {
        return this->m_uniqueAddress == other.m_uniqueAddress;
      }

      bool operator!=(Key const& other) const { return !(*this == other); }
    };

  private:
    struct ContextSharedState
    {
      std::shared_ptr<ContextSharedState> Parent;
      std::atomic<DateTime::rep> Deadline;
      Context::Key Key;
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
          : Deadline(ToDateTimeRepresentation((DateTime::max)())), Value(nullptr),
            ValueType(typeid(std::nullptr_t))
      {
      }

      explicit ContextSharedState(
          const std::shared_ptr<ContextSharedState>& parent,
          DateTime const& deadline)
          : Parent(parent), Deadline(ToDateTimeRepresentation(deadline)), Value(nullptr),
            ValueType(typeid(std::nullptr_t))
      {
      }

      template <class T>
      explicit ContextSharedState(
          const std::shared_ptr<ContextSharedState>& parent,
          DateTime const& deadline,
          Context::Key const& key,
          T value) // NOTE, should this be T&&
          : Parent(parent), Deadline(ToDateTimeRepresentation(deadline)), Key(key),
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
     * @brief Construct a new context with no deadline, and no value associated.
     */
    Context() : m_contextSharedState(std::make_shared<ContextSharedState>()) {}

    /**
     * @brief Copy constructor.
     */
    Context& operator=(const Context&) = default;

    /**
     * @brief Create a context with a deadline.
     *
     * @param deadline A point in time after which a context expires.
     *
     * @return A child context with deadline.
     */
    Context WithDeadline(DateTime const& deadline) const
    {
      return Context{std::make_shared<ContextSharedState>(m_contextSharedState, deadline)};
    }

    /**
     * @brief Create a context without a deadline, but with \p key and \p value associated with it.
     *
     * @param key A key to associate with this context.
     * @param value A value to associate with this context.
     *
     * @return A child context with no deadline and the \p key and \p value associated with it.
     */
    template <class T> Context WithValue(Key const& key, T&& value) const
    {
      return Context{std::make_shared<ContextSharedState>(
          m_contextSharedState, (DateTime::max)(), key, std::forward<T>(value))};
    }

    /**
     * @brief Get a deadline associated with this context or the branch of contexts this context
     * belongs to.
     *
     * @return A deadline associated with the context found; `Azure::DateTime::max()` value if a
     * specific value can't be found.
     */
    DateTime GetDeadline() const;

    /**
     * @brief Try to get a value associated with a \p key parameter within this context or the
     * branch of contexts this context belongs to.
     *
     * @param key A key associated with a context to find.
     * @param outputValue A reference to the value corresponding to the key to be set, if found
     * within the context tree.
     *
     * @return If found, returns true, with outputValue set to the value associated with the context
     * found; otherwise returns false.
     */
    template <class T> bool TryGetValue(Key const& key, T& outputValue) const
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
          outputValue = *reinterpret_cast<const T*>(ptr->Value.get());
          return true;
        }
      }
      return false;
    }

    /**
     * @brief Get a value associated with a \p key parameter within this context or the branch of
     * contexts this context belongs to.
     *
     * @param key A key associated with a context to find.
     *
     * @return A value associated with the context found; an empty value if a specific value can't
     * be found.
     */
    template <class T> const T& GetValue(Key const& key) const
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
      std::abort();
      // It should be expected that keys may not exist
      //  That implies we return T* and NOT a T&
    }

    /**
     * @brief Cancels the context.
     */
    void Cancel()
    {
      m_contextSharedState->Deadline
          = ContextSharedState::ToDateTimeRepresentation((DateTime::min)());
    }

    /**
     * @brief Check if the context is cancelled.
     * @return `true` if this context is cancelled, `false` otherwise.
     */
    bool IsCancelled() const { return GetDeadline() < std::chrono::system_clock::now(); }

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
