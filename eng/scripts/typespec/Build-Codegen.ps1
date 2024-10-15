# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

[CmdletBinding()]
param(
    [switch] $ForceInstall = $false,
    [switch] $Reinstall = $false
)

pushd
cd ..\..\
cd codegen.cpp

if ($Reinstall) {
    rm -Recurse -Force .\node_modules\ &&  rm .\package-lock.json
}

if ($ForceInstall) {
   npm install --force
} else {
   npm install
}

npm run build

cd ..\typespec-cpp

if ($Reinstall) {
    rm -Recurse -Force .\node_modules\ &&  rm .\package-lock.json
}

if ($ForceInstall) {
   npm install --force
} else {
   npm install
}

npm run build
popd
