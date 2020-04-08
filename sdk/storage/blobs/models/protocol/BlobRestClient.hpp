// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http.hpp"

namespace Azure { namespace Storage { namespace Blobs {

class BlobRestClient
{
public:
    class Container
    {
        public:
        static Azure::Core::Http::Request Create_CreateMessage (
            const Azure::Core::Http::Pipeline& pipeline,
            const Azure::Core::Http::Uri& resourceUri,
            const PublicAccessType& accessType,
            const std::string& version,
            const std::map<std::string, std::string>& metadata = std::map<std::string, std::string>(),
            const std::string& clientRequestID = std::string()
        );

        static Azure::Core::Http::Response<BlobContainerInfo> Create_CreateResponse (
            const Azure::Core::Http::Response& response
        );

        static Azure::Core::Http::Response<BlobContainerInfo> Create (
            const Azure::Core::Http::Pipeline& pipeline,
            const Azure::Core::Http::Uri& resourceUri,
            const PublicAccessType& accessType,
            const std::string& version,
            const std::map<std::string, std::string>& metadata = std::map<std::string, std::string>(),
            const std::string& clientRequestID = std::string()
        );
    };

    class BlockBlob
    {
        static Azure::Core::Http::Request StageBlock_CreateMessage (
            const Azure::Core::Http::Pipeline& pipeline,
            const Azure::Core::Http::Uri& resourceUri,
            const Azure::Core::Http::BodyBuffer* bodyBuffer,
            uint64_t length,
            const std::string& blockId,
            const std::string& version,
            const std::string& transactionalMd5 = std::string(),
            const std::string& transactionalCrc64 = std::string(),
            const std::string& leaseId = std::string(),
            const std::string& encryptionKey = std::string(),
            const std::string& encryptionKeySha256 = std::string(),
            const EncryptionAlgorithmType& type = EncryptionAlgorithmType.None,
            const std::string& encryptionScope = std::string(),
            const std::string& clientRequestID = std::string()
        );

        static Azure::Core::Http::Request StageBlock_CreateMessage (
            const Azure::Core::Http::Pipeline& pipeline,
            const Azure::Core::Http::Uri& resourceUri,
            const Azure::Core::Http::BodyStream* bodyStream,
            uint64_t length,
            const std::string& blockId,
            const std::string& version,
            const std::string& transactionalMd5 = std::string(),
            const std::string& transactionalCrc64 = std::string(),
            const std::string& leaseId = std::string(),
            const std::string& encryptionKey = std::string(),
            const std::string& encryptionKeySha256 = std::string(),
            const EncryptionAlgorithmType& type = EncryptionAlgorithmType.None,
            const std::string& encryptionScope = std::string(),
            const std::string& clientRequestID = std::string()
        );

        static Azure::Core::Http::Response<BlockInfo> StageBlock_CreateResponse (
            const Azure::Core::Http::Response& response
        );

        static Azure::Core::Http::Response<BlockInfo> StageBlock (
            const Azure::Core::Http::Pipeline& pipeline,
            const Azure::Core::Http::Uri& resourceUri,
            const Azure::Core::Http::BodyStream* bodyStream,
            uint64_t length,
            const std::string& blockId,
            const std::string& version,
            const std::string& transactionalMd5 = std::string(),
            const std::string& transactionalCrc64 = std::string(),
            const std::string& leaseId = std::string(),
            const std::string& encryptionKey = std::string(),
            const std::string& encryptionKeySha256 = std::string(),
            const EncryptionAlgorithmType& type = EncryptionAlgorithmType.None,
            const std::string& encryptionScope = std::string(),
            const std::string& clientRequestID = std::string()
        );

        static Azure::Core::Http::Response<BlockInfo> StageBlock (
            const Azure::Core::Http::Pipeline& pipeline,
            const Azure::Core::Http::Uri& resourceUri,
            const Azure::Core::Http::BodyBuffer* bodyBuffer,
            uint64_t length,
            const std::string& blockId,
            const std::string& version,
            const std::string& transactionalMd5 = std::string(),
            const std::string& transactionalCrc64 = std::string(),
            const std::string& leaseId = std::string(),
            const std::string& encryptionKey = std::string(),
            const std::string& encryptionKeySha256 = std::string(),
            const EncryptionAlgorithmType& type = EncryptionAlgorithmType.None,
            const std::string& encryptionScope = std::string(),
            const std::string& clientRequestID = std::string()
        );
    };

};

}}}
