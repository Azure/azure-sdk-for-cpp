# base image installs dev env for storage v12 beta10
FROM vhvb1989/storage12:preview10

# Get vcpkg and install storage blobs
RUN sudo git clone https://github.com/microsoft/vcpkg.git \
    && cd vcpkg \
    # VCPKG commit version for key vault keys 1.0.0-beta.1 - Storage beta 10 - Identity beta 5 - Core beta 8
    && sudo git checkout 414bec05f2a97cfc0ddb8e22fd4635dfe2a20ab8 \
    && sudo ./bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/vcpkg

RUN cd vcpkg \
    && sudo ./vcpkg install azure-storage-blobs-cpp
