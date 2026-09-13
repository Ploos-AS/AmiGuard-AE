#include <stdio.h>
#include <string.h>

#include "checksum.h"

static unsigned long crc32_update(unsigned long crc, unsigned char value)
{
    unsigned int bit;
    crc ^= (unsigned long)value;
    for (bit = 0; bit < 8; ++bit) {
        if (crc & 1UL)
            crc = (crc >> 1) ^ 0xEDB88320UL;
        else
            crc >>= 1;
    }
    return crc;
}

int amiguard_ae_checksum_crc32(const char *path, AmiGuardAEChecksumResult *out)
{
    FILE *fp;
    unsigned char buffer[512];
    size_t got;
    unsigned long crc = 0xFFFFFFFFUL;
    unsigned long final_crc;

    if (out == 0) return 0;
    out->ok = 0;
    out->checksum[0] = '\0';
    out->detail[0] = '\0';

    if (path == 0 || path[0] == '\0') {
        strcpy(out->detail, "invalid file path");
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == 0) {
        strcpy(out->detail, "cannot open file read-only");
        return 0;
    }

    while ((got = fread(buffer, 1U, sizeof(buffer), fp)) != 0U) {
        size_t i;
        for (i = 0; i < got; ++i)
            crc = crc32_update(crc, buffer[i]);
    }

    if (ferror(fp)) {
        fclose(fp);
        strcpy(out->detail, "file read failed");
        return 0;
    }

    fclose(fp);
    final_crc = crc ^ 0xFFFFFFFFUL;
    sprintf(out->checksum, "%08lX", final_crc);
    strcpy(out->detail, "CRC32");
    out->ok = 1;
    return 1;
}
