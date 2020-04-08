// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Blobs {

class BlobLeaseClient
{
public:
    BlobLeaseClient(const BlobClient& blobClient, std::string leaseId) : m_blobClient(&blobClient), m_beaseId(std::move(leaseId)) {}

    BlobLeaseClient(const BlobContainerClient& blobContainerClient, std::string leaseId) : m_blobContainerClient(&blobContainerClient), m_leaseId(std::move(leaseId)) {}

    static std::string CreateUniqueLeaseId();

    Azure::Core::Http::Response<BlobLease> Acquire(int duration, const AcquireLeaseOptions& options = AcquireLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Renew(const RenewLeaseOptions& options = RenewLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Release(const ReleaseLeaseOptions& options = ReleaseLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Change(const std::string& proposedId, const ChangeLeaseOptions& options = ChangeLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Break(const BreakLeaseOptions& options = BreakLeaseOptions());

private:
    const BlobContainerClient* m_blobContainerClient = nullptr;
    const BlobClient* m_blobClient = nullptr;
    std::string m_leaseId;
};

}}}
