#ifndef PHYPHOXBLE_COMMON
#define PHYPHOXBLE_COMMON

#include "Arduino.h"

static const char *phyphoxBleExperimentServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleExperimentCharacteristicUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";
[[maybe_unused]] static const char *phyphoxBleExperimentControlCharacteristicUUID = "cddf0003-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleEventCharacteristicUUID = "cddf0004-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleDataServiceUUID = "cddf1001-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleDataCharacteristicUUID = "cddf1002-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleConfigCharacteristicUUID = "cddf1003-30f7-4671-8b43-5e40ba53514a";

static const char *deviceName = "phyphox-Arduino";

struct phyphoxBleCrc32
{
    static void generate_table(uint32_t(&table)[256])
    {
        uint32_t polynomial = 0xEDB88320;
        for (uint32_t i = 0; i < 256; i++)
        {
            uint32_t c = i;
            for (size_t j = 0; j < 8; j++)
            {
                if (c & 1) {
                    c = polynomial ^ (c >> 1);
                }
                else {
                    c >>= 1;
                }
            }
            table[i] = c;
        }
    }

    static uint32_t update(uint32_t (&table)[256], uint32_t initial, const uint8_t* buf, size_t len)
    {
        uint32_t c = initial ^ 0xFFFFFFFF;
        const uint8_t* u = static_cast<const uint8_t*>(buf);
        for (size_t i = 0; i < len; ++i)
        {
            c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
        }
        return c ^ 0xFFFFFFFF;
    }
};

static int64_t swap_int64( int64_t val ){
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

#endif
