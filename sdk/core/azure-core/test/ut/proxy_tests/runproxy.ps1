# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

docker run --rm -d -p 3128:3128 squid-local
docker run --rm -d -p 3129:3129 squid-local.passwd
