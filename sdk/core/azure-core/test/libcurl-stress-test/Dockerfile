# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

ARG targetTest
ARG build

# copy the tagrget binary
ADD $targetTest ./$targetTest
RUN chmod +x ./$targetTest

# install the mem check tool 
RUN apt-get update -y
RUN apt-get install valgrind -y