// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <type_traits>

namespace Azure { namespace Core {

  struct ValueBase
  {
    virtual ~ValueBase() {}
  };

  /*
   *  @brief ContextValue exists as a substitute for variant which isn't available until C++17
   */
  class ContextValue {
    enum class ContextValueType
    {
      Undefined,
      Bool,
      Int,
      StdString,
      UniquePtr
    };

    ContextValueType m_contextValueType;
    union
    {
      bool m_b; // if m_contextValueType == ContextValueType::Bool
      int m_i; // if m_contextValueType == ContextValueType::Int
      std::string m_s; // if m_contextValueType == ContextValueType::StdString
      std::unique_ptr<ValueBase> m_p; // if m_contextValueType == ContextValueType::UniquePtr
    };

  public:
    ContextValue() noexcept : m_contextValueType(ContextValueType::Undefined) {}
    ContextValue(bool b) noexcept : m_contextValueType(ContextValueType::Bool), m_b(b) {}
    ContextValue(int i) noexcept : m_contextValueType(ContextValueType::Int), m_i(i) {}
    ContextValue(const std::string& s) : m_contextValueType(ContextValueType::StdString), m_s(s) {}
    ContextValue(std::string&& s) noexcept
        : m_contextValueType(ContextValueType::UniquePtr), m_s(std::move(s))
    {
    }
    template <
        class DerivedFromValueBase,
        typename std::
            enable_if<std::is_convertible<DerivedFromValueBase*, ValueBase*>::value, int>::type
        = 0>
    ContextValue(std::unique_ptr<DerivedFromValueBase>&& p) noexcept
        : m_contextValueType(ContextValueType::UniquePtr), m_p(std::move(p))
    {
    }

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

    template <class ExpectedType> const ExpectedType& Get() const noexcept;

    ContextValueType Alternative() const noexcept { return m_contextValueType; }
  };

  template <> inline const bool& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::Bool)
    {
      abort();
    }
    return m_b;
  }

  template <> inline const int& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::Int)
    {
      abort();
    }
    return m_i;
  }

  template <> inline const std::string& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::StdString)
    {
      abort();
    }
    return m_s;
  }

  template <> inline const std::unique_ptr<ValueBase>& ContextValue::Get() const noexcept
  {
    if (m_contextValueType != ContextValueType::UniquePtr)
    {
      abort();
    }
    return m_p;
  }

  class Context {
  public:
    using time_point = std::chrono::system_clock::time_point;

  private:
    struct ContextSharedState
    {
      std::shared_ptr<ContextSharedState> const Parent;
      std::chrono::milliseconds CancelAtMsecSinceEpoch;
      std::string const Key;
      ContextValue const Value;

      static constexpr std::chrono::milliseconds ToMsecSinceEpoch(time_point time)
      {
        return time - time_point();
      }

      static constexpr time_point FromMsecSinceEpoch(std::chrono::milliseconds msec)
      {
        return time_point() + msec;
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

  public:
    Context() : m_contextSharedState(std::make_shared<ContextSharedState>()) {}

    Context& operator=(const Context&) = default;

    Context WithDeadline(time_point cancelWhen) const
    {
      return Context{std::make_shared<ContextSharedState>(
          m_contextSharedState, cancelWhen, std::string(), ContextValue{})};
    }

    Context WithValue(const std::string& key, ContextValue&& value) const
    {
      return Context{std::make_shared<ContextSharedState>(
          m_contextSharedState, time_point::max(), key, std::move(value))};
    }

    time_point CancelWhen() const;

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

    void Cancel()
    {
      m_contextSharedState->CancelAtMsecSinceEpoch
          = ContextSharedState::ToMsecSinceEpoch(time_point::min());
    }

    void ThrowIfCanceled() const
    {
      if (CancelWhen() < std::chrono::system_clock::now())
      {
        // TODO: Runtime Exc
        throw;
      }
    }
  };

  Context& GetApplicationContext();
  Context const& GetApplicationContext() const;
}} // namespace Azure::Core
