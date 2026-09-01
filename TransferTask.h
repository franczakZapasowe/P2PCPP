#pragma once
#include "ChunkHeader.h"
#include <vector>
#pragma pack(push,1)
struct TransferTask {
    ChunkHeader header;
    std::vector<char>data;
};
#pragma pack(pop)