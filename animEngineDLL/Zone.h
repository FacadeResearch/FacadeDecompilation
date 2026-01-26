#pragma once
#include <cstdint>

struct Zone {
    uint8_t unknownData[80]; // We dont know what the first 80 padded bytes even do, but each zone is like 92 bytes when initialized so
    int32_t someFlag; //looks like a flag?
    uint8_t padding[8]; //this is the remaining 8 bytes of the unknown object
};