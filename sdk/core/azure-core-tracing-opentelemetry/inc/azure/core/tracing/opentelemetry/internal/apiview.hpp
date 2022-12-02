// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

// This file is supposed to be included only when generating files for ApiView.
// All the declarations here are only sufficient for the ApiView generation to not fail.

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
    static nostd::shared_ptr<TracerProvider> GetTracerProvider();
  };
} // namespace trace
} // namespace opentelemetry