# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

[CmdletBinding()]
param()

Get-ChildItem -Exclude *.cpp,*.tsp,*.ps1,*.yaml,.clang-format,generated | Remove-Item -Recurse -Force
