// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/common/global_state.hpp"
#include <cassert>
#include <mutex>
#include <stdexcept>

#include <azure_c_shared_utility/platform.h>

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  GlobalStateHolder::GlobalStateHolder()
  {
    if (platform_init())
    {
      throw std::runtime_error("Could not initialize platform.");
    }
  }

  GlobalStateHolder::~GlobalStateHolder() { platform_deinit(); }

  GlobalStateHolder* GlobalStateHolder::GlobalStateInstance()
  {
    static GlobalStateHolder globalState;
    return &globalState;
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail
