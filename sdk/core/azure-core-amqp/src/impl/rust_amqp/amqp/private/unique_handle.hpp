// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/internal/unique_handle.hpp>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  template <typename T> struct UniqueHandleHelper;

  template <typename T> using UniqueHandle = Core::_internal::UniqueHandle<T, UniqueHandleHelper>;

}}}} // namespace Azure::Core::Amqp::_detail
