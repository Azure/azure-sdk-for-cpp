# Copyright (c) Microsoft Corporation. All rights reserved.
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04 as build
# FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

# install the mem check tool along side the other deps
RUN apt-get update -y
RUN apt-get install -y gcc cmake make g++ git zip unzip build-essential pkg-config wget curl valgrind
RUN wget -O vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz
RUN mkdir /opt/vcpkg
RUN tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg
RUN /opt/vcpkg/bootstrap-vcpkg.sh
RUN ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg

ADD . /src
WORKDIR /build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DBUILD_TRANSPORT_CURL=ON /src
RUN cmake --build . --target azure-core-libcurl-stress-test

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

RUN apt-get update -y
RUN apt-get install -y valgrind
WORKDIR /

# copy the target binary
COPY --from=build ./build/sdk/core/azure-core/test/libcurl-stress-test/azure-core-libcurl-stress-test ./azure-core-libcurl-stress-test
RUN chmod +x ./azure-core-libcurl-stress-test

#CMD valgrind --tool=memcheck -s ./azure-core-libcurl-stress-test
# this should be run by the scenarios matrix , run valgrind only when needed since it impacts performance and resources. 