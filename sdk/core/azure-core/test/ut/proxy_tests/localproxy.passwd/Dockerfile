#@# vim: set filetype=dockerfile:
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# DisableDockerDetector "This was used for testing in the past. If such testing can be supported in the future this container will be used again."
FROM azsdkengsys.azurecr.io/mirror/ubuntu/squid
LABEL maintainer "Larry Osterman<github.com/LarryOsterman>"

ENV NGINX_VERSION 1.23.1

##
# dependent packages for docker build
##

WORKDIR /tmp

RUN apt-get -y update 

##
# application deployment
##

WORKDIR /

COPY ./squid.conf /tmp/squid.conf
COPY ./proxypasswd /tmp/proxypasswd

EXPOSE 3129

STOPSIGNAL SIGTERM

ENTRYPOINT squid -f /tmp/squid.conf && sleep 10 && tail -f /var/log/squid/access.log
