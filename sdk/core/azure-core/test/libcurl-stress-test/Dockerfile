# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

RUN su
# install the mem check tool 
RUN apt-get update -y
RUN apt-get install gcc cmake make g++ git zip unzip build-essential pkg-config wget curl valgrind -y
RUN wget -O vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz
RUN mkdir /opt/vcpkg
RUN tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg
RUN /opt/vcpkg/bootstrap-vcpkg.sh
RUN ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg

RUN git clone https://github.com/Azure/azure-sdk-for-cpp.git 
WORKDIR /azure-sdk-for-cpp/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DBUILD_TRANSPORT_CURL=ON ..
RUN cmake --build . --target azure-core-libcurl-stress-test 

# copy the tagrget binary
RUN cp /azure-sdk-for-cpp/build/sdk/core/azure-core/test/libcurl-stress-test/azure-core-libcurl-stress-test ./azure-core-libcurl-stress-test
RUN chmod +x ./azure-core-libcurl-stress-test
