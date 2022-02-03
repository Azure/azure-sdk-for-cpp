# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Defines utility functions to create build targets for CI.
#

macro(create_per_service_target_build target service)

    file(APPEND ${CMAKE_BINARY_DIR}/${service}-targets-build.txt "${target}\n")

endmacro()
