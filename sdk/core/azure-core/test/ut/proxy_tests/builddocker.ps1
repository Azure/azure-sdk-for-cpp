# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

pushd .\localproxy
docker build -t squid-local .
popd

pushd .\localproxy.passwd
docker build -t squid-local.passwd .
popd

pushd .\remoteproxy
docker build -t squid-remote .
popd

pushd .\remoteproxy.passwd
docker build -t squid-remote.passwd .
popd