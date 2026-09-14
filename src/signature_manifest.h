#ifndef AMIGUARD_AE_SIGNATURE_MANIFEST_H
#define AMIGUARD_AE_SIGNATURE_MANIFEST_H

#define AMIGUARD_AE_MANIFEST_ALGORITHM_MAX 16
#define AMIGUARD_AE_MANIFEST_KEYID_MAX 64
#define AMIGUARD_AE_MANIFEST_DATABASE_MAX 256
#define AMIGUARD_AE_MANIFEST_CRC32_MAX 9
#define AMIGUARD_AE_MANIFEST_SIGNATURE_MAX 129
#define AMIGUARD_AE_MANIFEST_DETAIL_MAX 128

typedef struct AmiGuardAESignatureManifest {
    char algorithm[AMIGUARD_AE_MANIFEST_ALGORITHM_MAX];
    char key_id[AMIGUARD_AE_MANIFEST_KEYID_MAX];
    unsigned long sequence;
    char database[AMIGUARD_AE_MANIFEST_DATABASE_MAX];
    char crc32[AMIGUARD_AE_MANIFEST_CRC32_MAX];
    char signature_hex[AMIGUARD_AE_MANIFEST_SIGNATURE_MAX];
} AmiGuardAESignatureManifest;

int amiguard_ae_signature_manifest_load(const char *path,
                                        AmiGuardAESignatureManifest *out,
                                        char *detail,
                                        unsigned long detail_size);

#endif
