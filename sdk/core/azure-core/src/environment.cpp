// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(AZ_BUILD_TESTING)

#include "azure/core/internal/environment.hpp"

#include <mutex>
#include <shared_mutex>

using namespace Azure::Core::_internal;

std::atomic<bool> Environment::g_isOverridden(false);

namespace {
std::shared_timed_mutex g_mutex;
Environment::GetEnvCallback g_getEnv(nullptr);
} // namespace

Environment::GetEnvCallback::result_type Environment::OverriddenGetEnv(char const* varName)
{
  std::shared_lock<std::shared_timed_mutex> lock(g_mutex);
  return g_getEnv(varName);
}

void Environment::Override(Environment::GetEnvCallback now)
{
  std::unique_lock<std::shared_timed_mutex> lock(g_mutex);
  g_getEnv = now;
  g_isOverridden = now != nullptr;
}

#endif
