#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Common {
  namespace _detail {

    /**
     * uAMQP and azure-c-shared-util require that the platform_init and platform_uninit functions be
     * called before using the various API functions.
     *
     * The GlobalState class maintains a singleton static local variable using [static local
     * variables](https://en.cppreference.com/w/cpp/language/storage_duration#Static_local_variables),
     * also known as "Magic Statics". This ensures that we initialize only once.
     *
     */

    class GlobalState {
    public:
      GlobalState();
      ~GlobalState();

      static GlobalState* GlobalStateInstance()
      {
        static auto globalState = new GlobalState();
        return globalState;
      };
    };
}}}}}} // namespace Azure::Core::_internal::Amqp::Common::_detail
