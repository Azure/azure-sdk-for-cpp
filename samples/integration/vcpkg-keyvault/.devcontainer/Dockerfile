# base image installs keyvault beta1 from vcpkg only
FROM vhvb1989/keyvault:beta1

RUN cd vcpkg \
    && sudo ./vcpkg install azure-identity-cpp

ENV VCPKG_DEFAULT_TRIPLET=x64-linux
