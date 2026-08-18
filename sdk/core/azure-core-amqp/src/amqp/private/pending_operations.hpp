// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <utility>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

#if ENABLE_UAMQP
  // uAMQP stops the poll of a connection in these two states, so nothing
  // completes an operation that is in flight after this point.
  inline bool ConnectionStateEndsPendingOperations(_internal::ConnectionState state)
  {
    return state == _internal::ConnectionState::Error || state == _internal::ConnectionState::End;
  }
#endif // ENABLE_UAMQP

  // The operations that wait on one connection.
  //
  // A sender and a receiver wait on a queue that only the polling thread fills,
  // so a connection that dies leaves that wait with nothing to end it.
  //
  // A waiter runs on the polling thread while that thread holds the connection
  // lock, so a waiter may only push into its own queue. A waiter that takes the
  // connection lock deadlocks. The lock order is the connection mutex, then the
  // registry mutex, then the queue mutex.
  class PendingOperationRegistry final {
  public:
    using Waiter = std::function<void(Models::_internal::AmqpError const&)>;

    // Keeps one waiter alive. A caller that came back on its own leaves no
    // waiter behind, because this object removes it.
    class Registration final {
    public:
      Registration() = default;
      ~Registration() { Reset(); }

      Registration(Registration const&) = delete;
      Registration& operator=(Registration const&) = delete;

      Registration(Registration&& other) noexcept : m_registry{other.m_registry}, m_id{other.m_id}
      {
        other.m_registry = nullptr;
      }

      Registration& operator=(Registration&& other) noexcept
      {
        if (this != &other)
        {
          Reset();
          m_registry = other.m_registry;
          m_id = other.m_id;
          other.m_registry = nullptr;
        }
        return *this;
      }

    private:
      friend class PendingOperationRegistry;

      Registration(PendingOperationRegistry* registry, std::uint64_t id)
          : m_registry{registry}, m_id{id}
      {
      }

      void Reset()
      {
        if (m_registry)
        {
          m_registry->Unregister(m_id);
          m_registry = nullptr;
        }
      }

      PendingOperationRegistry* m_registry{nullptr};
      std::uint64_t m_id{0};
    };

    // A caller that registers after the connection died gets the latched error
    // at once. The Event Hubs recover path closes the old sender after the
    // connection is gone, so without that call the close waits forever.
    Registration Register(Waiter waiter)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      std::uint64_t const id = m_nextId++;
      auto const inserted = m_operations.emplace(id, Entry{std::move(waiter), m_woken});
      if (m_woken)
      {
        inserted.first->second.Waiter(m_latchedError);
      }
      return Registration{this, id};
    }

    // A connection goes to Error and then to End, so this runs more than once
    // and each waiter runs one time. The latched error serves a later caller.
    void WakeAll(Models::_internal::AmqpError const& error)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      if (!m_woken)
      {
        m_woken = true;
        m_latchedError = error;
      }
      for (auto& operation : m_operations)
      {
        if (!operation.second.Fired)
        {
          operation.second.Fired = true;
          operation.second.Waiter(m_latchedError);
        }
      }
    }

    // The number of live Registration objects.
    std::size_t PendingCount() const
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      return m_operations.size();
    }

  private:
    struct Entry final
    {
      PendingOperationRegistry::Waiter Waiter;
      bool Fired{false};
    };

    void Unregister(std::uint64_t id)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_operations.erase(id);
    }

    mutable std::mutex m_mutex;
    std::map<std::uint64_t, Entry> m_operations;
    std::uint64_t m_nextId{0};
    bool m_woken{false};
    Models::_internal::AmqpError m_latchedError;
  };

}}}} // namespace Azure::Core::Amqp::_detail
