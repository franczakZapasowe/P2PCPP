#pragma once
#include <cstdint>
#pragma pack(push, 1)
struct FileTransferHeader {
    uint32_t id;
    uint64_t size;
    char name [256];
};
#pragma pack(pop)