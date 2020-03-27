// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage {

class BlobLeaseClient
{
public:
    BlobLeaseClient(const BlobClient& blobClient, std::string leaseId) : blob_client(&blobClient), lease_id(std::move(leaseId)) {}

    BlobLeaseClient(const BlobContainerClient& blobContainerClient, std::string leaseId) : blob_container_client(&blobContainerClient), lease_id(std::move(leaseId)) {}

    static std::string CreateUniqueLeaseId();

    Azure::Core::Http::Response<BlobLease> Acquire(int duration, const AcquireLeaseOptions& options = AcquireLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Renew(const RenewLeaseOptions& options = RenewLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Release(const ReleaseLeaseOptions& options = ReleaseLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Change(const std::string& proposedId, const ChangeLeaseOptions& options = ChangeLeaseOptions());

    Azure::Core::Http::Response<BlobLease> Break(const BreakLeaseOptions& options = BreakLeaseOptions());

private:
    const BlobContainerClient* blob_container_client = nullptr;
    const BlobClient* blob_client = nullptr;
    std::string lease_id;
};

}}
