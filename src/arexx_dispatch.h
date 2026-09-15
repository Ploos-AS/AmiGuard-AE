#ifndef AMIGUARD_AE_AREXX_DISPATCH_H
#define AMIGUARD_AE_AREXX_DISPATCH_H

/*
 * Large enough for the longest bounded M4.3 quarantine reply:
 * fixed status text + 16-character quarantine ID + a full 255-byte
 * quarantine path + NUL. Keep this larger than QUARANTINE_PATH_MAX rather
 * than silently truncating the machine-readable ARexx result.
 */
#define AMIGUARD_AE_RESULT_MAX 384

typedef struct AmiGuardAERexxResult {
    long rc;
    char result[AMIGUARD_AE_RESULT_MAX];
} AmiGuardAERexxResult;

void amiguard_ae_arexx_dispatch(const char *command, AmiGuardAERexxResult *out);

#endif
