#ifndef AMIGUARD_AE_CHECKSUM_H
#define AMIGUARD_AE_CHECKSUM_H

#define AMIGUARD_AE_CHECKSUM_TEXT_MAX 16
#define AMIGUARD_AE_CHECKSUM_DETAIL_MAX 96

typedef struct AmiGuardAEChecksumResult {
    int ok;
    char checksum[AMIGUARD_AE_CHECKSUM_TEXT_MAX];
    char detail[AMIGUARD_AE_CHECKSUM_DETAIL_MAX];
} AmiGuardAEChecksumResult;

int amiguard_ae_checksum_crc32(const char *path, AmiGuardAEChecksumResult *out);

#endif
