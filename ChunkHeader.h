#pragma once
#include <cstdint>
#pragma pack(push,1)
struct ChunkHeader {
    uint32_t file_id;       // musimy wiedziec jakiego pliku to czesc
    uint32_t offset;        // przesniecie w bajtach od poczatku pliku
    uint32_t chunk_size;    // ile surowych bajtow danych nastapi zaraz po tym naglowku
 };
#pragma pack (pop)