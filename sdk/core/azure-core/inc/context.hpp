// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <assert.h>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <type_traits>

constexpr char not_found = 0;

namespace azure { namespace core {

  struct ValueBase
  {
    virtual ~ValueBase() {}
  };

  /*
   *  @brief ContextValue exists as a substitute for variant which isn't available until C++17
   */
  class ContextValue
  {
    char active;
    union {
      bool b; // if active == 1
      int i; // if active == 2
      std::string s; // if active == 3
      std::unique_ptr<ValueBase> p; // if active == 4
    };

  public:
    ContextValue() noexcept : active(0) {}
    ContextValue(bool b) noexcept : active(1), b(b) {}
    ContextValue(int i) noexcept : active(2), i(i) {}
    ContextValue(const std::string& s) : active(3), s(s) {}
    ContextValue(std::string&& s) noexcept : active(3), s(std::move(s)) {}
    template <
        class DerivedFromValueBase,
        typename std::
            enable_if<std::is_convertible<DerivedFromValueBase*, ValueBase*>::value, int>::type
        = 0>
    ContextValue(std::unique_ptr<DerivedFromValueBase>&& p) noexcept : active(4), p(std::move(p))
    {}

    ContextValue(ContextValue&& other) : active(other.active)
    {
      switch (active)
      {
        case 1:
          b = other.b;
          break;
        case 2:
          i = other.i;
          break;
        case 3:
          ::new (&s) std::string(std::move(other.s));
          break;
        case 4:
          ::new (&p) std::unique_ptr<ValueBase>(std::move(other.p));
          break;
      }
    }

    ~ContextValue()
    {
      switch (active)
      {
        case 3:
          s.~basic_string();
          break;
        case 4:
          p.~unique_ptr<ValueBase>();
          break;
      }
    }

    ContextValue& operator=(const ContextValue& other) = delete;

    template <class ExpectedType> const ExpectedType& get() const noexcept;

    char alternative() const noexcept { return active; }
  };

  template <> const bool& ContextValue::get() const noexcept
  {
    if (active != 1)
    {
      abort();
    }
    return b;
  }

  template <> const int& ContextValue::get() const noexcept
  {
    if (active != 2)
    {
      abort();
    }
    return i;
  }

  template <> const std::string& ContextValue::get() const noexcept
  {
    if (active != 3)
    {
      abort();
    }
    return s;
  }

  template <> const std::unique_ptr<ValueBase>& ContextValue::get() const noexcept
  {
    if (active != 4)
    {
      abort();
    }
    return p;
  }

  class Context
  {
  public:
    using time_point = std::chrono::system_clock::time_point;
    using ContextValue = ContextValue;

  private:
    struct ContextSharedState
    {
      std::shared_ptr<ContextSharedState> parent;
      time_point cancel_at; // or something like this
      std::string key;
      ContextValue value;

      explicit ContextSharedState() : cancel_at(time_point::max()) {}

      explicit ContextSharedState(
          const std::shared_ptr<ContextSharedState>& parent,
          time_point cancel_at,
          const std::string& key,
          ContextValue&& value)
          : parent(parent), cancel_at(cancel_at), key(key), value(std::move(value))
      {}
    };

    std::shared_ptr<ContextSharedState> impl_;

    explicit Context(std::shared_ptr<ContextSharedState> impl) : impl_(std::move(impl)) {}

  public:
    Context() : impl_(std::make_shared<ContextSharedState>()) {}

    Context& operator=(const Context&) = default;

    Context with_deadline(time_point cancel_here)
    {
      return Context{
          std::make_shared<ContextSharedState>(impl_, cancel_here, std::string(), ContextValue{})};
    }

    Context with_value(const std::string& key, ContextValue&& value)
    {
      return Context{
          std::make_shared<ContextSharedState>(impl_, time_point::max(), key, std::move(value))};
    }

    time_point cancel_when()
    {
      auto result = time_point::max();
      for (auto ptr = impl_; ptr; ptr = ptr->parent)
      {
        if (result > ptr->cancel_at)
        {
          result = ptr->cancel_at;
        }
      }

      return result;
    }

    const ContextValue& operator[](const std::string& key)
    {
      if (!key.empty())
      {
        for (auto ptr = impl_; ptr; ptr = ptr->parent)
        {
          if (ptr->key == key)
          {
            return ptr->value;
          }
        }
      }

      static ContextValue empty;
      return empty;
    }

    void cancel() { impl_->cancel_at = time_point::min(); }
  };

  Context& get_application_context()
  {
    static Context ctx;
    return ctx;
  }
}} // namespace azure::core
