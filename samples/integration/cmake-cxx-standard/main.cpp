// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/context.hpp>

#if defined(_MSVC_LANG)
static_assert(_MSVC_LANG >= EXPECTED_CXX_VERSION, "The requested C++ standard was not preserved.");
#else
static_assert(__cplusplus >= EXPECTED_CXX_VERSION, "The requested C++ standard was not preserved.");
#endif

int main()
{
  Azure::Core::Context context;
  return context.IsCancelled() ? 1 : 0;
}
