FROM debian:10

# This Dockerfile adds a non-root 'vscode' user with sudo access. However, for Linux,
# this user's GID/UID must match your local user UID/GID to avoid permission issues
# with bind mounts. Update USER_UID / USER_GID if yours is not 1000. See
# https://aka.ms/vscode-remote/containers/non-root-user for details.
ARG USERNAME=azure-sdk-for-cpp
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG PORT=4000

# Install packages as root
USER root

# Install needed packages and setup non-root user. Use a separate RUN statement to add your own dependencies.
RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && LANG=C LC_ALL=C apt-get -y install --no-install-recommends \
    apt-utils \
    dialog \
    sudo \
    #
    # Install vim, git, process tools, lsb-release
    git \
    openssh-client \
    less \
    #
    # Azure SDK for C++ dev env
    make \
    cmake \
    ninja-build \
    build-essential \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    libxml2-dev \
    gdb \
    # clang format 10 req
    gnupg2 \
    wget \
    ca-certificates \
    # vcpkg reqs
    curl \
    zip \
    unzip \
    tar \
    pkg-config \

    #
    # Add en_US.UTF-8 locale
    && echo 'en_US.UTF-8 UTF-8' >> /etc/locale.gen \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && echo 'deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main' | tee -a /etc/apt/sources.list \
    && echo 'deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main' | tee -a /etc/apt/sources.list \
    && apt-get update \
    && apt-get -y install --no-install-recommends clang-format-10 \
    #
    # Create a non-root user to use if preferred - see https://aka.ms/vscode-remote/containers/non-root-user.
    && groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # Add sudo support for the non-root user
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME\
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    #
    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN cd / \
    && git clone https://github.com/microsoft/vcpkg.git \
    && cd vcpkg \
    && ./bootstrap-vcpkg.sh \
    && ./vcpkg install curl 

# Switch back to the non-root user
USER ${USERNAME}
