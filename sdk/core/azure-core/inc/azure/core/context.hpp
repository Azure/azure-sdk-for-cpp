// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Context for canceling long running operations.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <new> //For the non-allocating placement new
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Azure { namespace Core {

  /**
   * @brief A base abstract class for the `std::unique_ptr` value representation in
   * #Azure::Core::ContextValue.
   *
   */
  struct ValueBase
  {
    virtual ~ValueBase() {}
  };

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
   * @brief Represents a value that is associated with context.
   * @remark Exists as a substitute for variant which isn't available until C++17.
   */
  class ContextValue {
  public:
    /**
     * @brief A type of context value.
     */
    enum class ContextValueType
    {
      Undefined, ///< Undefined.
      Bool, ///< `bool`.
      Int, ///< `int`.
      StdString, ///< `std::string`
      UniquePtr ///< `std::unique_ptr<ValueBase>`
    };

  private:
    ContextValueType m_contextValueType;
    union
    {
      bool m_b; // if m_contextValueType == ContextValueType::Bool
      int m_i; // if m_contextValueType == ContextValueType::Int
      std::string m_s; // if m_contextValueType == ContextValueType::StdString
      std::unique_ptr<ValueBase> m_p; // if m_contextValueType == ContextValueType::UniquePtr
    };

  public:
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

    /**
     * @brief Create a context value with undefined value type.
     */
    ContextValue() noexcept : m_contextValueType(ContextValueType::Undefined) {}

    /**
     * @brief Create a context value with `bool` value type.
     *
     * @param b Boolean value.
     */
    ContextValue(bool b) noexcept : m_contextValueType(ContextValueType::Bool), m_b(b) {}

    /**
     * @brief Create a context value with `int` value type.
     *
     * @param i Integer value.
     */
    ContextValue(int i) noexcept : m_contextValueType(ContextValueType::Int), m_i(i) {}

    /**
     * @brief Create a context value with `std::string` value type.
     *
     * @param s String value.
     */
    ContextValue(const std::string& s) : m_contextValueType(ContextValueType::StdString), m_s(s) {}

    /**
     * @brief Create a context value with `std::string` value type.
     *
     * @param s String value represented as 0-terminated C-string.
     */
    ContextValue(const char* s) : m_contextValueType(ContextValueType::StdString), m_s(s) {}

    /**
     * @brief Create a context value with `std::string` value type, using `std::move` semantics.
     *
     * @param s String value.
     */
    ContextValue(std::string&& s)
        : m_contextValueType(ContextValueType::StdString), m_s(std::move(s))
    {
    }

    /**
     * @brief Create a context value with `std::unique_ptr<Azure::Core::ValueBase>` value type.
     *
     * @param p Smart pointer to #Azure::Core::ValueBase.
     */
    ContextValue(std::unique_ptr<ValueBase>&& p) noexcept
        : m_contextValueType(ContextValueType::UniquePtr), m_p(std::move(p))
    {
    }

    /**
     * @brief Move constructor.
     *
     * @param other An rvalue reference to another instance of #Azure::Core::ContextValue.
     */
    ContextValue(ContextValue&& other) noexcept : m_contextValueType(other.m_contextValueType)
    {
      switch (m_contextValueType)
      {
        case ContextValueType::Bool:
          m_b = other.m_b;
          break;
        case ContextValueType::Int:
          m_i = other.m_i;
          break;
        case ContextValueType::StdString:
          ::new (&m_s) std::string(std::move(other.m_s));
          break;
        case ContextValueType::UniquePtr:
          ::new (&m_p) std::unique_ptr<ValueBase>(std::move(other.m_p));
          break;
        case ContextValueType::Undefined:
          break;
      }
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    /**
     * @brief #Azure::Core::ContextValue destructor.
     */
    ~ContextValue()
    {
      switch (m_contextValueType)
      {
        case ContextValueType::StdString:
          m_s.~basic_string();
          break;
        case ContextValueType::UniquePtr:
          m_p.~unique_ptr<ValueBase>();
          break;
        case ContextValueType::Bool:
        case ContextValueType::Int:
        case ContextValueType::Undefined:
          break;
      }
    }

    ContextValue& operator=(const ContextValue& other) = delete;

    /**
     * @brief Get the context value.
     *
     * @tparam ExpectedType The type of value to get.
     */
    template <class ExpectedType> const ExpectedType& Get() const noexcept;

    /**
     * @brief Get the context value type.
     */
    ContextValueType Alternative() const noexcept { return m_contextValueType; }
  };

  /**
   * @brief Get the context value type as boolean (`bool`).
   */
  template <> inline const bool& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::Bool)
    {
      abort();
    }
    return m_b;
  }

  /**
   * @brief Get the context value type as integer (`int`).
   */
  template <> inline const int& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::Int)
    {
      abort();
    }
    return m_i;
  }

  /**
   * @brief Get the context value type as string (`std::string`).
   */
  template <> inline const std::string& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::StdString)
    {
      abort();
    }
    return m_s;
  }

  /**
   * @brief Get the context value type as a pointer (`std::unique_ptr<ValueBase>`).
   */
  template <> inline const std::unique_ptr<ValueBase>& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::UniquePtr)
    {
      abort();
    }
    return m_p;
  }

  /**
   * @brief A context is a node within a tree that represents expiration times and key/value pairs.
   */
  class Context {
  public:
    /**
     * @brief A type used to provide a point in time for context expiration.
     */
    using time_point = std::chrono::system_clock::time_point;

  private:
    struct ContextSharedState
    {
      std::shared_ptr<ContextSharedState> Parent;
      std::atomic_int64_t CancelAtMsecSinceEpoch;
      std::string Key;
      ContextValue Value;

      static constexpr int64_t ToMsecSinceEpoch(time_point time)
      {
        return static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(time - time_point()).count());
      }

      static constexpr time_point FromMsecSinceEpoch(int64_t msec)
      {
        return time_point() + static_cast<std::chrono::milliseconds>(msec);
      }

      explicit ContextSharedState() : CancelAtMsecSinceEpoch(ToMsecSinceEpoch(time_point::max())) {}

      explicit ContextSharedState(
          const std::shared_ptr<ContextSharedState>& parent,
          time_point cancelAt,
          const std::string& key,
          ContextValue&& value)
          : Parent(parent), CancelAtMsecSinceEpoch(ToMsecSinceEpoch(cancelAt)), Key(key),
            Value(std::move(value))
      {
      }
    };

    std::shared_ptr<ContextSharedState> m_contextSharedState;

    explicit Context(std::shared_ptr<ContextSharedState> impl)
        : m_contextSharedState(std::move(impl))
    {
    }

    time_point CancelWhen() const;

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
     * @param cancelWhen A point in time after which a context expires.
     *
     * @return A child context with expiration.
     */
    Context WithDeadline(time_point cancelWhen) const
    {
      return Context{std::make_shared<ContextSharedState>(
          m_contextSharedState, cancelWhen, std::string(), ContextValue{})};
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
    Context WithValue(const std::string& key, ContextValue&& value) const
    {
      return Context{std::make_shared<ContextSharedState>(
          m_contextSharedState, time_point::max(), key, std::move(value))};
    }

    /**
     * @brief Get a value associated with a \p key parameter within this context or the branch of
     * contexts this context belongs to.
     *
     * @param key A key assiciated with a context to find.
     *
     * @return A value associated with the context found; an empty value if a specific value can't
     * be found.
     */
    const ContextValue& operator[](const std::string& key) const
    {
      if (!key.empty())
      {
        for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
        {
          if (ptr->Key == key)
          {
            return ptr->Value;
          }
        }
      }

      static ContextValue empty;
      return empty;
    }

    /**
     * @brief Check whether the context has a key matching \p key parameter in it itself, or in the
     * branch the context belongs to.
     *
     * @param key A key assiciated with a context to find.
     *
     * @return `true` if this context, or the tree branch this context belongs to has a \p key
     * associsted with it. `false` otherwise.
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
      m_contextSharedState->CancelAtMsecSinceEpoch
          = ContextSharedState::ToMsecSinceEpoch(time_point::min());
    }

    /**
     * @brief Check if the context is cancelled.
     * @return `true` if this context is cancelled, `false` otherwise.
     */
    bool IsCancelled() const { return CancelWhen() < std::chrono::system_clock::now(); }

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
  };

  /**
   * @brief Get the application context (root).
   */
  Context& GetApplicationContext();
}} // namespace Azure::Core
