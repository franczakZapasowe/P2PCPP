#pragma once
#include "ChunkHeader.h"
#include <vector>
struct TransferTask {
    ChunkHeader header;
    std::vector<char>data;
};