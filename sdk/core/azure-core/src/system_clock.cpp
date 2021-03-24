// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/internal/system_clock.hpp"

#include <mutex>
#include <shared_mutex>

using namespace Azure::Core::_internal;

std::atomic<bool> SystemClock::g_isOverridden(false);

namespace {
std::shared_timed_mutex g_mutex;
std::function<std::chrono::system_clock::time_point()> g_now(nullptr);
} // namespace

std::chrono::system_clock::time_point SystemClock::OverriddenNow()
{
  std::shared_lock<std::shared_timed_mutex> lock(g_mutex);
  return g_now();
}

void SystemClock::Override(std::function<std::chrono::system_clock::time_point()> now)
{
  std::unique_lock<std::shared_timed_mutex> lock(g_mutex);
  g_now = now;
  g_isOverridden = now != nullptr;
}
