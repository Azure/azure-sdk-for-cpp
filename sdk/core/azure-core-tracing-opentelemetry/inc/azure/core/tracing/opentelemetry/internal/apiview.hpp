// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace opentelemetry {
namespace nostd {
  template <typename> struct shared_ptr
  {
  };
} // namespace nostd
namespace trace {
  struct TracerProvider;
  struct Provider
  {
    nostd::shared_ptr<TracerProvider> GetTracerProvider() { return {}; }
  };
} // namespace trace
} // namespace opentelemetry
