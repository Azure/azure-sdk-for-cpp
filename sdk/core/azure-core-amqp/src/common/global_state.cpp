// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/common/global_state.hpp"
#include <cassert>
#include <mutex>
#include <stdexcept>

#include <azure_c_shared_utility/platform.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Common {
  namespace _detail {

    GlobalState::GlobalState()
    {
      if (platform_init())
      {
        throw std::runtime_error("Could not initialize platform.");
      }
    }

    GlobalState::~GlobalState() { platform_deinit(); }

}}}}}} // namespace Azure::Core::_internal::Amqp::Common::_detail
