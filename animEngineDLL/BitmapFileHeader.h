#include <cstdint>
#pragma pack(push, 1)
struct BitmapFileHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offBits;
};
#pragma pack(pop)