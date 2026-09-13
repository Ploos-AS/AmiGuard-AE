#ifndef AMIGUARD_AE_IDENTIFY_H
#define AMIGUARD_AE_IDENTIFY_H

typedef struct AmiGuardAEIdentifyResult {
    int ok;
    char type[32];
    char detail[96];
} AmiGuardAEIdentifyResult;

int amiguard_ae_identify_file(const char *path, AmiGuardAEIdentifyResult *out);

#endif
