# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

# copy the tagrget binary
COPY ./bin/libcurl-stress-test/azure-core-libcurl-stress-test ./azure-core-libcurl-stress-test
RUN chmod +x ./azure-core-libcurl-stress-test

# install the mem check tool 
RUN apt-get update -y
RUN apt-get install valgrind -y

