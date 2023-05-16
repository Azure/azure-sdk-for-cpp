// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  /**
   * uAMQP and azure-c-shared-util require that the platform_init and platform_uninit functions be
   * called before using the various API functions.
   *
   * The GlobalState class maintains a singleton static local variable using [static local
   * variables](https://en.cppreference.com/w/cpp/language/storage_duration#Static_local_variables),
   * also known as "Magic Statics". This ensures that we initialize only once.
   *
   */

  class GlobalStateHolder {
    GlobalStateHolder();
    ~GlobalStateHolder();

  public:
    static GlobalStateHolder* GlobalStateInstance();
  };
}}}}} // namespace Azure::Core::Amqp::Common::_detail
