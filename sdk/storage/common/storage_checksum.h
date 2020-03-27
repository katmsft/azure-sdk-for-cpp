// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace Azure { namespace Storage { namespace Common {

enum ChecksumType
{
    None,
    MD5,
    SHA256,
    CRC64
};

class Checksum
{
public:
    Checksum(ChecksumType type, std::string value) : type(type), value(std::move(value)) {}
    Checksum(ChecksumType type, uint64_t value)
    {
        if (type != ChecksumType::Crc64)
        {
            throw StorageException("Invalid checksum type");
        }
        type = type;
        // TODO: convert int value to string notation
    }

    static const Checksum& None()
    {
        static Checksum c(ChecksumType::None, std::string());
        return c;
    }

    ChecksumType Type()
    {
        return type;
    }

    const std::string& Value()
    {
        return value;
    }

private:
    ChecksumType type;
    std::string value;
};
