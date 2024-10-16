# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

[CmdletBinding()]
param(
    [switch] $ForceInstall = $false,
    [switch] $Reinstall = $false
)

# install the global packages
npm install -g @microsoft/rush typescript autorest @typespec/compiler @azure-tools/typespec-client-generator-cli @azure-tools/typespec-azure-rulesets

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
