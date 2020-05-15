// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>
#include <vector>

#include "common/internal/protocol/rest_client_utility.hpp"
#include "common/xml_wrapper.hpp"

namespace Azure
{
namespace Storage
{
namespace Blobs
{

enum class AccessTier
{
  Unknown,
  P1,
  P2,
  P3,
  P4,
  P6,
  P10,
  P15,
  P20,
  P30,
  P40,
  P50,
  P60,
  P70,
  P80,
  Hot,
  Cool,
  Archive,
};

enum class BlockType
{
  Committed = 0,
  Uncommitted,
  Latest
};

struct BlockInfo : public Common::ResponseInfo
{
  bool ServerEncrypted;
  std::string ContentMd5 = std::string();
  std::string ContentCrc64 = std::string();
  std::string EncryptionKeySha256 = std::string();
};

struct BlobContainerInfo : public Common::ResponseInfo
{
  std::string Etag = std::string();
  std::string LastModified = std::string();
};

struct BlobContentInfo : public Common::ResponseInfo
{
  std::string Etag;
  std::string LastModified;
  std::string ContentMD5;
  std::string ContentCRC64;
  uint64_t SequenceNumber;
  std::string EncryptionKeySha256;
  bool ServerEncrypted;
};

class BlobRestClient
{
  const static std::string k_HEADER_X_MS_BLOB_TYPE;
  const static std::string k_HEADER_BLOCK_BLOB;
  const static std::string k_HEADER_PAGE_BLOB;
  const static std::string k_HEADER_APPEND_BLOB;
  const static std::string k_HEADER_X_MS_BLOB_SEQUENCE_NUMBER;
  const static std::string k_QUERY_CONTAINER;
  const static std::string k_QUERY_BLOCK_LIST;
  const static std::string k_QUERY_BLOCK;
  const static std::string k_QUERY_BLOB;
  const static std::string k_QUERY_BLOCK_ID;

public:
  class Container
  {
    const static std::string k_HEADER_MS_BLOB_PUBLIC_ACCESS;

  public:
    enum class PublicAccessType
    {
      Container = 0,
      Blob,
      Anonymous
    };

    struct CreateOptions : public Common::RequestOptions
    {
      PublicAccessType AccessType = PublicAccessType::Anonymous;
      std::map<std::string, std::string> Metadata = std::map<std::string, std::string>();
    };

    static azure::core::http::Request CreateConstructRequest(
        /*const*/ std::string& url,
        /*const*/ CreateOptions& options)
    {
      auto request = azure::core::http::Request(azure::core::http::HttpMethod::Put, url);
      if (options.AccessType == PublicAccessType::Blob)
      {
        request.addHeader(k_HEADER_MS_BLOB_PUBLIC_ACCESS, k_QUERY_BLOB);
      }
      else if (options.AccessType == PublicAccessType::Container)
      {
        request.addHeader(k_HEADER_MS_BLOB_PUBLIC_ACCESS, k_QUERY_CONTAINER);
      }

      ApplyBasicHeaders(options, request);

      Common::AddMetadata(options.Metadata, request);

      request.addQueryParameter(Common::k_QUERY_RESTYPE, k_QUERY_CONTAINER);

      return request;
    };

    // TODO: should return azure::core::http::Response<BlobContainerInfo> instead
    static BlobContainerInfo CreateParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlobContainerInfo info;
      if (/*TODO: when response errored*/ response.getStatusCode() >= 300U)
      {
      }
      else
      {
        // TODO: ContainerUrl initialization.
        info.Etag = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_ETAG);
        info.LastModified
            = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_LAST_MODIFIED);
        ParseBasicResponseHeaders(response.getHeaders(), info);
      }

      return info;
    }

    // TODO: should return azure::core::http::Response<BlobContainerInfo> instead
    static BlobContainerInfo Create(
        // TODO: Context and Pipeline should go here
        /*const*/ std::string& url,
        /*const*/ CreateOptions& options)
    {
      return CreateParseResponse(
          azure::core::http::Client::send(CreateConstructRequest(url, options)));
    };
  };

  class BlockBlob
  {
    const static std::string k_XML_TAG_BLOCK_LIST;
    const static std::string k_XML_TAG_COMMITTED;
    const static std::string k_XML_TAG_UNCOMMITTED;
    const static std::string k_XML_TAG_LATEST;

  public:

    struct StageBlockOptions : public Common::BodiedRequestOptions
    {
      std::string BlockId;
      std::string ContentMd5 = std::string();
      std::string ContentCrc64 = std::string();
      std::string LeaseId = std::string();
      std::string EncryptionKey = std::string();
      std::string EncryptionKeySha256 = std::string();
      std::string EncryptionScope = std::string();
    };

    static azure::core::http::Request StageBlockConstructRequest(
        /*const*/ std::string& url,
        /*const*/ StageBlockOptions& options)
    {
      azure::core::http::Request request(azure::core::http::HttpMethod::Put, url);
      request.addQueryParameter(k_QUERY_BLOCK_ID, options.BlockId);
      request.addQueryParameter(Common::k_QUERY_COMP, k_QUERY_BLOCK);
      request.setBodyBuffer(options.BodyBuffer);
      request.setBodyStream(options.BodyStream);
      ApplyBasicHeaders(options, request);
      return request;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo StageBlockParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlockInfo info;
      info.ContentMd5 = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_CONTENT_MD5);
      info.ContentCrc64
          = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_X_MS_CONTENT_CRC64);
      ParseBasicResponseHeaders(response.getHeaders(), info);
      return info;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo StageBlock(
        // TODO: Context and Pipeline should go here
        /*const*/ std::string& url,
        /*const*/ StageBlockOptions& options)
    {
      return StageBlockParseResponse(
          azure::core::http::Client::send(StageBlockConstructRequest(url, options)));
    }

    struct CommitBlockListOptions : public Common::RequestOptions
    {
      std::vector<std::pair<BlockType, std::string>> BlockList
          = std::vector<std::pair<BlockType, std::string>>();
      std::string ContentMd5 = std::string();
      std::string ContentCrc64 = std::string();
      std::string LeaseId = std::string();
      std::string EncryptionKey = std::string();
      std::string EncryptionKeySha256 = std::string();
      std::string EncryptionScope = std::string();
    };

    static azure::core::http::Request CommitBlockListConstructRequest(
        /*const*/ std::string& url,
        /*const*/ CommitBlockListOptions& options)
    {
      azure::core::http::Request request(azure::core::http::HttpMethod::Put, url);
      request.addQueryParameter(Common::k_QUERY_COMP, k_QUERY_BLOCK_LIST);
      ApplyBasicHeaders(options, request);
      // TODO: avoid the data copy here.
      block_list_writer writer;
      std::string body = writer.write(options.BlockList);
      // TODO: Investigate why not copying the ending \0 would result in request body sending more
      // data than expected.
      uint8_t* buffer = new uint8_t[body.size() + 1];
      memcpy_s(buffer, body.size() + 1, body.c_str(), body.size() + 1);
      azure::core::http::BodyBuffer* bodyBuffer
          = new azure::core::http::BodyBuffer(buffer, body.size());
      request.setBodyBuffer(bodyBuffer);
      return request;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo CommitBlockListParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlockInfo info;
      info.ContentMd5 = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_CONTENT_MD5);
      info.ContentCrc64
          = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_X_MS_CONTENT_CRC64);
      ParseBasicResponseHeaders(response.getHeaders(), info);
      return info;
    }

    // TODO: should return azure::core::http::Response<BlockInfo> instead
    static BlockInfo CommitBlockList(
        // TODO: Context and Pipeline should go here
        /*const*/ std::string& url,
        /*const*/ CommitBlockListOptions& options)
    {
      // TODO Manage the memory that is allocated when constructing the request.
      return CommitBlockListParseResponse(
          azure::core::http::Client::send(CommitBlockListConstructRequest(url, options)));
    }

  private:
    class block_list_writer : public Azure::Storage::Common::XML::xml_writer
    {
    public:
      block_list_writer() {}

      std::string write(const std::vector<std::pair<BlockType, std::string>>& blockList)
      {
        std::ostringstream outstream;
        initialize(outstream);

        write_start_element(k_XML_TAG_BLOCK_LIST);

        for (auto block = blockList.cbegin(); block != blockList.cend(); ++block)
        {
          std::string tag;
          switch (block->first)
          {
            case BlockType::Committed:
              tag = k_XML_TAG_COMMITTED;
              break;

            case BlockType::Uncommitted:
              tag = k_XML_TAG_UNCOMMITTED;
              break;

            case BlockType::Latest:
              tag = k_XML_TAG_LATEST;
              break;
          }

          write_element(tag, block->second);
        }

        finalize();
        return outstream.str();
      }
    };
  };

  class PageBlob
  {
  public:
    struct CreateOptions : public Common::RequestOptions
    {
      std::string ContentEncoding;
      std::string ContentLanguage;
      std::string CacheControl;
      std::string ContentDiposition;
      std::map<std::string, std::string> Metadata;
      AccessTier Tier;
    };

    static azure::core::http::Request CreateConstructRequest(
        const std::string& url,
        const CreateOptions& options)
    {
      auto request = azure::core::http::Request(azure::core::http::HttpMethod::Put, url);
      ApplyBasicHeaders(options, request);

      request.addHeader(k_HEADER_X_MS_BLOB_TYPE, k_HEADER_PAGE_BLOB);

      Common::AddMetadata(options.Metadata, request);

      if (options.Tier != AccessTier::Unknown)
      {
        // TODO: wait until enum to string
        // request.addHeader(Common::k_HEADER_X_MS_ACCESS_TIER, );
      }

      return request;
    };

    // TODO: should return azure::core::http::Response<BlobContainerInfo> instead
    static BlobContentInfo CreateParseResponse(/*const*/ azure::core::http::Response& response)
    {
      BlobContentInfo info;
      if (response.getStatusCode() >= 300U)
      {
        /* TODO: when response errored */
      }
      else
      {
        ParseBasicResponseHeaders(response.getHeaders(), info);
        info.Etag = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_ETAG);
        info.LastModified
            = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_LAST_MODIFIED);
        info.ContentMD5
            = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_CONTENT_MD5);
        info.ContentCRC64
            = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_X_MS_CONTENT_CRC64);
        info.SequenceNumber = std::stoull(Common::GetHeaderValue(
            response.getHeaders(), k_HEADER_X_MS_BLOB_SEQUENCE_NUMBER));
        info.ServerEncrypted
            = Common::GetHeaderValue(response.getHeaders(), Common::k_HEADER_X_MS_SERVER_ENCRYPTED)
            == "true";
        info.EncryptionKeySha256 = Common::GetHeaderValue(
            response.getHeaders(), Common::k_HEADER_X_MS_ENCRYPTION_KEY_SHA256);
      }

      return info;
    }

    // TODO: should return azure::core::http::Response<BlobContainerInfo> instead
    static BlobContentInfo Create(
        // TODO: Context and Pipeline should go here
        const std::string& url,
        const CreateOptions& options)
    {
      return CreateParseResponse(
          azure::core::http::Client::send(CreateConstructRequest(url, options)));
    };

    struct UploadPagesOptions : public Common::BodiedRequestOptions
    {
    };

    struct ClearPagesOptions : public Common::BodiedRequestOptions
    {
    };

    struct UploadPagesFromUriOptions : public Common::BodiedRequestOptions
    {
    };

    struct GetPageRangesOptions : public Common::RequestOptions
    {
    };

    struct GetPageRangesDiffOption : public Common::RequestOptions
    {
    };

    struct ResizeOptions : public Common::RequestOptions
    {
    };

    struct UpdateSequenceNumberOptions : public Common::RequestOptions
    {
    };

    struct CopyIncrementalOptions : public Common::RequestOptions
    {
    };
  };
};

} // namespace Blobs
} // namespace Storage
} // namespace Azure