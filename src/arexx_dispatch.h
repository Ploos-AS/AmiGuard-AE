#ifndef AMIGUARD_AE_AREXX_DISPATCH_H
#define AMIGUARD_AE_AREXX_DISPATCH_H

#define AMIGUARD_AE_RESULT_MAX 256

typedef struct AmiGuardAERexxResult {
    long rc;
    char result[AMIGUARD_AE_RESULT_MAX];
} AmiGuardAERexxResult;

void amiguard_ae_arexx_dispatch(const char *command, AmiGuardAERexxResult *out);

#endif
