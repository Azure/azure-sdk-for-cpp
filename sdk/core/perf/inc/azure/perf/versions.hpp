// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Print the `=== Versions ===` block at end of a run.
 *
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Perf {

  /**
   * @brief Print the `=== Versions ===` block to stdout, mirroring the Go perf framework
   * output. Lists the compiler/toolchain, optional CMake-injected vcpkg port versions,
   * and any caller-supplied package versions.
   *
   * @param injectedVersions Additional `(name, version)` pairs to include in the block.
   * Storage perf executables already print their own `VCPKG_..._VERSION` lines for the
   * perf-automation tool; pass anything else worth recording here.
   */
  void PrintVersionsBlock(
      std::vector<std::pair<std::string, std::string>> const& injectedVersions = {});

}} // namespace Azure::Perf
