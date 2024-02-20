# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04 as build
# FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

# install the mem check tool along side the other deps
RUN apt-get update -y
RUN apt-get install -y gcc cmake make g++ git zip unzip build-essential pkg-config wget curl valgrind python3
RUN wget -O vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz
RUN mkdir /opt/vcpkg
RUN tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg
RUN /opt/vcpkg/bootstrap-vcpkg.sh
RUN ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg

ADD . /src
WORKDIR /build
# During CMake generate step VCPKG runs in manifest mode, as such it will sync the packages to the level 
# of the hash specified in src/azure-sdk-for-cpp/cmake-modules/AzureVcpkg.cmake in the VCPKG_COMMIT_STRING
# environment variable thus the packages we run with are not the latest versions but the ones the code
# was developed against. If the builtin-baseline is specified in the vcpkg file then that is the top most
# version of the packages that will be fetched.
# So when building from root we need to match the two values. When not building from root if the vcpkg file
# does not specify a baseline the value set in the cmake file will ensure that we are at the desired level.
RUN --mount=type=cache,target=/root/.cache/vcpkg/archives --mount=type=cache,target=/root/.m2 cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DBUILD_TRANSPORT_CURL=ON /src 
RUN cmake --build . --target azure-messaging-eventhubs-stress-test

FROM mcr.microsoft.com/mirror/docker/library/ubuntu:22.04

RUN apt-get update -y
RUN apt-get install -y valgrind
WORKDIR /

# copy the target binary
COPY --from=build ./build/sdk/eventhubs/azure-messaging-eventhubs/test/eventhubs-stress-test/src/azure-messaging-eventhubs-stress-test ./azure-messaging-eventhubs-stress-test
RUN chmod +x ./azure-messaging-eventhubs-stress-test

CMD ./azure-messaging-eventhubs-stress-test
# this should be run by the scenarios matrix , run valgrind only when needed since it impacts performance and resources. 
